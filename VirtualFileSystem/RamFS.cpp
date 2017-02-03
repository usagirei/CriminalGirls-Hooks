#include "RamFS.h"
#include "stdafx.h"
#include "pkzip.h"
#include "console.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <codecvt>

#include <assert.h>
#include <zlib.h>

void ZIPInflatePreProcessor(const RamFS::Entry& entry, char* buffer);

std::wstring StringToWideString(const std::string& str, int codePage = CP_ACP)
{
	int cstrl = str.length() + 1;
	auto temp = new wchar_t[cstrl];
	MultiByteToWideChar(codePage, 0, str.c_str(), -1, temp, cstrl);
	auto wstr = std::wstring(temp);
	delete[] temp;
	return wstr;
}


void RamFS::Tracker::Read(PVOID pBuffer, DWORD nBytesToRead, PDWORD nBytesRead)
{
	RamFS &vfs = *this->FileSystem;

	bool callback = false;

	*nBytesRead = 0;
	while (nBytesToRead > 0) {
		if (this->Position < vfs.BinaryHeaderSize) {
			const uint64_t srcOffset = this->Position;
			const uint64_t srcRemainder = vfs.BinaryHeaderSize - srcOffset;
			const uint32_t toRead = min(srcRemainder, nBytesToRead);

			uint8_t* dst = (uint8_t*)pBuffer;
			uint8_t* src = ((uint8_t*)vfs.BinaryHeader) + srcOffset;

			memcpy(dst, src, toRead);

			*nBytesRead += toRead;
			nBytesToRead -= toRead;
			this->Position += toRead;
		}
		else {
			bool found = false;
			for (int i = 0; i < vfs.Entries.size(); i++) {
				RamFS::Entry &rEntry = *(vfs.Entries.at(i));

				PS3FS_HEADER_ENTRY &bEntry = *rEntry.BinaryEntry;
				uint64_t srcOffset = this->Position;
				bool afterStart = srcOffset >= bEntry.Offset;
				bool beforeEnd = srcOffset < (bEntry.Offset + rEntry.FileSize);
				bool between = afterStart && beforeEnd;
				if (between) {
					if (!callback) {
						for (int i = 0; i < vfs.OnFileRead.size(); i++) {
							(*vfs.OnFileRead.at(i))(bEntry.Name);
						}
						callback = true;
					}

					if (rEntry.Mounted)
						tcout << "Open Mounted File: " << bEntry.Name << "\n";

					found = true;
					const int64_t vOffset = srcOffset - bEntry.Offset;
					int32_t toRead;

					// TODO: Caching Pointers, LRU?
					// Processed Read
					if (rEntry.FileProcessor) {
						const int64_t srcRemainder = rEntry.ProcessedFileSize - vOffset;
						toRead = min(srcRemainder, nBytesToRead);

						if (vfs.ProcessingArea.RequireInvalidate(rEntry)) {
							vfs.ProcessingArea.Invalidate(rEntry);
							rEntry.FileProcessor(rEntry, vfs.ProcessingArea.DataBuffer);
						}
						memcpy((uint8_t*)pBuffer + *nBytesRead, vfs.ProcessingArea.DataBuffer + vOffset, toRead);
					}
					// Standard Read
					else {
						const int64_t srcRemainder = rEntry.FileSize - vOffset;
						toRead = min(srcRemainder, nBytesToRead);

						const int64_t fOffset = rEntry.SourceOffset + vOffset;

						FILE* fp;
						int error = _wfopen_s(&fp, rEntry.SourceName.c_str(), L"rb");
						_fseeki64(fp, fOffset, FILE_BEGIN);
						int read = fread((uint8_t*)pBuffer + *nBytesRead, 1, toRead, fp);
						fclose(fp);
						if (read != toRead)
							tcout << "Misread?\n";
					}

					*nBytesRead += toRead;
					nBytesToRead -= toRead;
					this->Position += toRead;
					break;
				}
			}
			if (!found)
				return;
		}
	}
}

uint64_t RamFS::Tracker::Seek(DWORD nOffset, DWORD eMoveMethod)
{
	switch (eMoveMethod)
	{
		case FILE_BEGIN:
			this->Position = nOffset;
			break;
		case FILE_CURRENT:
			this->Position += nOffset;
			break;
		case FILE_END:
			this->Position = this->FileSystem->TotalSize + nOffset;
			break;
	}
	return this->Position;
}

RamFS::Tracker *RamFS::CreateTracker()
{
	RamFS::Tracker* tracker = new RamFS::Tracker();
	tracker->Position = 0;
	tracker->FileSystem = this;
	return tracker;
}

RamFS::Entry * RamFS::FindEntry(const std::string &rName)
{
	const int maxLen = sizeof(PS3FS_HEADER_ENTRY::Name) / sizeof(char);
	if (rName.length() > maxLen)
		return nullptr;

	int hash = RamFS::Entry::HashFunc(rName);
	auto found = std::find_if(this->Entries.begin(), this->Entries.end(), [hash](const RamFS::Entry *e) {
		return e->Hash == hash;
	});
	if (found == this->Entries.end())
		return nullptr;
	return *found;
}

RamFS::Entry* RamFS::CreateEntry(const std::string &rName) {

	const int maxLen = sizeof(PS3FS_HEADER_ENTRY::Name) / sizeof(char);
	if (rName.length() > maxLen)
		return nullptr;

	RamFS::Entry *rEntry = FindEntry(rName);
	if (!rEntry) {
		rEntry = new RamFS::Entry(rName);

		std::transform(rEntry->FileName.begin(), rEntry->FileName.end(), rEntry->FileName.begin(), tolower);

		this->Entries.push_back(rEntry);
	}
	return rEntry;
}

void RamFS::MountDat(const std::string &pName) {

	FILE* inDatFile;
	int error = fopen_s(&inDatFile, pName.c_str(), "rb");

	if (error != 0) {
		std::cout << con::fgCol<con::Red>;
		std::cout << "Failed to Open Dat File: " << pName << "\n";
		std::cout << con::fgCol<con::Gray>;
		return;
	}

	uint64_t nFiles = 0;
	fseek(inDatFile, 8, FILE_BEGIN);
	fread(&nFiles, 1, 8, inDatFile);

	for (int i = 0; i < nFiles; ++i) {
		PS3FS_HEADER_ENTRY datEntry;
		fread(&datEntry, 1, sizeof(PS3FS_HEADER_ENTRY), inDatFile);

		std::string entryName = datEntry.Name;
		RamFS::Entry *rEntry = this->CreateEntry(entryName);
		if (!rEntry) {
			std::cout << con::fgCol<con::Red>;
			std::cout << "Failed to Create Entry: " << entryName << "\n";
			std::cout << con::fgCol<con::Gray>;
			continue;
		}

		rEntry->SourceName = StringToWideString(pName);

		rEntry->SourceOffset = datEntry.Offset;
		rEntry->FileSize = datEntry.Size;
		rEntry->Mounted = false;

		rEntry->FileProcessor = nullptr;
		rEntry->ProcessedFileSize = datEntry.Size;
	}

	fclose(inDatFile);
}

void RamFS::MountDir(const std::string &dirName)
{
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	hFind = FindFirstFileA((dirName + "*").c_str(), &data);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	std::cout << "Mounting Directory: " << dirName << "\n";

	do {
		std::string fileName = dirName + data.cFileName;
		std::ifstream stream(fileName);
		if (stream.good()) {
			stream.seekg(0, std::ios::end);
			int fSz = stream.tellg();

			std::string entryName = data.cFileName;
			RamFS::Entry *rEntry = this->CreateEntry(entryName);
			if (!rEntry) {
				std::cout << con::fgCol<con::Red>;
				std::cout << "Failed to Create Entry: " << entryName << "\n";
				std::cout << con::fgCol<con::Gray>;
				continue;
			}

			rEntry->FileProcessor = nullptr;
			rEntry->ProcessedFileSize = fSz;

			rEntry->SourceName = StringToWideString(fileName);
			rEntry->SourceOffset = 0;

			rEntry->FileSize = fSz;
			rEntry->Mounted = true;
		}
	} while (FindNextFileA(hFind, &data));

	FindClose(hFind);
}

void RamFS::MountZip(const std::string & zipFileName)
{
	FILE* zipFile;
	int error = fopen_s(&zipFile, zipFileName.c_str(), "rb");

	std::cout << "Mounting ZIP File: " << zipFileName << "\n";

	int signature;
	fread(&signature, 4, 1, zipFile);
	if (signature != ZIP_FILE_ENTRY_SIGNATURE) {
		std::cout << con::fgCol<con::Red>;
		std::cout << "Not a ZIP File \n";
		std::cout << con::fgCol<con::Gray>;
		return;
	}

	fseek(zipFile, 0, SEEK_END);
	int zipSize = ftell(zipFile);

	int curPos = zipSize - 22;
	bool found = false;
	while (curPos > 0) {
		fseek(zipFile, curPos, SEEK_SET);
		fread(&signature, 4, 1, zipFile);
		if (signature == ZIP_END_CD_SIGNATURE) {
			fseek(zipFile, curPos, SEEK_SET);
			found = true;
			break;
		}
		curPos -= 5;
	}

	if (!found) {
		std::cout << con::fgCol<con::Red>;
		std::cout << "Couldn't locate ZIP Central Directory\n";
		std::cout << con::fgCol<con::Gray>;
		return;
	}

	ZIPENDLOCATOR eocd(zipFile);

	fseek(zipFile, eocd.DirectoryOffset, SEEK_SET);

	for (int i = 0; i < eocd.EntriesInDirectory; ++i) {
		ZIPDIRENTRY dirEntry(zipFile);

		if (dirEntry.Compression != COMPTYPE::STORED && dirEntry.Compression != COMPTYPE::DEFLATE) {
			std::cout << con::fgCol<con::Red>;
			tcout << "Compression Type Unsupported. Use Store/Deflate Mode\n";
			std::cout << con::fgCol<con::Gray>;
			continue;
		}

		// Directory
		if (dirEntry.CompressedSize == 0)
			continue;

		int nextHeader = ftell(zipFile);

		fseek(zipFile, dirEntry.HeaderOffset, SEEK_SET);
		ZIPFILERECORD fileRecord(zipFile, false);

		std::string entryName = fileRecord.FileName;
		entryName = entryName.substr(entryName.rfind('/') + 1);

		RamFS::Entry* rEntry = this->CreateEntry(entryName);
		if (!rEntry) {
			std::cout << con::fgCol<con::Red>;
			std::cout << "Failed to Create Entry: " << entryName << "\n";
			std::cout << con::fgCol<con::Gray>;
		}
		else {
			rEntry->SourceName = StringToWideString(zipFileName);
			rEntry->SourceOffset = ftell(zipFile) - fileRecord.CompressedSize;

			rEntry->FileSize = fileRecord.CompressedSize;
			rEntry->Mounted = true;

			rEntry->ProcessedFileSize = fileRecord.UncompressedSize;
			if (fileRecord.Compression == COMPTYPE::DEFLATE) {
				rEntry->FileProcessor = ZIPInflatePreProcessor;
			}
			else {
				rEntry->FileProcessor = nullptr;
			}
		}

		fseek(zipFile, nextHeader, SEEK_SET);
	}

	fclose(zipFile);
}

void RamFS::Build()
{
	//std::random_shuffle(this->Entries.begin(), this->Entries.end());
	std::sort(this->Entries.begin(), this->Entries.end(), [](const RamFS::Entry *a, const RamFS::Entry *b) {
		return a->FileName < b->FileName;
	});

	if (this->BinaryHeader)
		this->BinaryHeader->Dealloc();

	this->BinaryHeader = PS3FS_HEADER::Prealloc(this->Entries.size(), &this->BinaryHeaderSize);
	this->TotalSize = this->BinaryHeaderSize;
	for (int i = 0; i < this->Entries.size(); ++i) {
		RamFS::Entry* rEntry = this->Entries.at(i);
		PS3FS_HEADER_ENTRY* bEntry = &this->BinaryHeader->Entries[i];
		rEntry->BinaryEntry = bEntry;

		strcpy(bEntry->Name, rEntry->FileName.c_str());
		if (rEntry->FileProcessor)
			bEntry->Size = rEntry->ProcessedFileSize;
		else
			bEntry->Size = rEntry->FileSize;
		bEntry->Offset = this->TotalSize;
		this->TotalSize += bEntry->Size;
	}
}

void ZIPInflatePreProcessor(const RamFS::Entry & entry, char * buffer)
{
	const int CHUNK = 4096;
	unsigned char* inChunk = new unsigned char[CHUNK];
	int ret;
	unsigned char* sBuffer = (unsigned char*)buffer;

	FILE* fp = _wfopen(entry.SourceName.c_str(), L"rb");
	fseek(fp, entry.SourceOffset, SEEK_SET);

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, -MAX_WBITS);

	int read = 0;
	int maxRead = entry.FileSize;
	int wrote = 0;
	int maxWrite = entry.ProcessedFileSize;

	do
	{
		int toRead = min(CHUNK, maxRead - read);
		strm.avail_in = fread(inChunk, 1, toRead, fp);
		read += strm.avail_in;

		if (ferror(fp)) {
			(void)inflateEnd(&strm);
			return;
		}
		if (strm.avail_in == 0)
			break;

		strm.next_in = inChunk;
		do {
			strm.avail_out = CHUNK;
			strm.next_out = sBuffer;

			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);

			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					fclose(fp);
					(void)inflateEnd(&strm);
					return;
			}

			int have = CHUNK - strm.avail_out;
			wrote += have;
			sBuffer = (unsigned char*)(buffer + wrote);

		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);
	(void)inflateEnd(&strm);

	delete[] inChunk;
	fclose(fp);
};
