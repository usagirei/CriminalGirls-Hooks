#include "RamFS.h"
#include "stdafx.h"

#include <iostream>


void RamFS::Close(RamFS * pFileSystem)
{
	if (!pFileSystem)
		return;

	for (int i = 0; i < pFileSystem->NumEntries; i++) {
		if (pFileSystem->Entries[i].FileName)
			delete pFileSystem->Entries[i].FileName;
	}

	if (pFileSystem->BinaryHeader)
		pFileSystem->BinaryHeader->Dealloc();

	if (pFileSystem->Entries)
		delete pFileSystem->Entries;
	delete pFileSystem;
}

void RamFS::Tracker::Read(PVOID pBuffer, DWORD nBytesToRead, PDWORD nBytesRead)
{
	RamFS* vfs = this->FileSystem;

	bool callback = false;

	*nBytesRead = 0;
	while (nBytesToRead > 0) {
		if (this->Position < vfs->BinaryHeaderSize) {
			const uint64_t srcOffset = this->Position;
			const uint64_t srcRemainder = vfs->BinaryHeaderSize - srcOffset;
			const uint32_t toRead = min(srcRemainder, nBytesToRead);

			uint8_t* dst = (uint8_t*)pBuffer;
			uint8_t* src = ((uint8_t*)vfs->BinaryHeader) + srcOffset;

			memcpy(dst, src, toRead);

			*nBytesRead += toRead;
			nBytesToRead -= toRead;
			this->Position += toRead;
		}
		else {
			bool found = false;
			for (int i = 0; i < vfs->NumEntries; i++) {
				RamFS::Entry* rEntry = &vfs->Entries[i];
				PS3FS_HEADER_ENTRY* bEntry = rEntry->BinaryEntry;
				uint64_t srcOffset = this->Position;
				bool afterStart = srcOffset >= bEntry->Offset;
				bool beforeEnd = srcOffset < (bEntry->Offset + rEntry->FileSize);
				bool between = afterStart && beforeEnd;
				if (between) {
					if (!callback) {
						if (vfs->OnFileRead) {
							for (int i = 0; i < vfs->OnFileRead->size(); i++) {
								(*vfs->OnFileRead->at(i))(bEntry->Name);
							}
						}
						callback = true;
					}

					if (rEntry->Mounted)
						tcout << "Open Mounted File: " << bEntry->Name << "\n";

					found = true;
					const int64_t vOffset = srcOffset - bEntry->Offset;
					const int64_t fOffset = rEntry->BaseOffset + vOffset;
					const int64_t srcRemainder = rEntry->FileSize - vOffset;
					const int32_t toRead = min(srcRemainder, nBytesToRead);

					// TODO: Caching Pointers, LRU?
					FILE* fp;
					int error = _wfopen_s(&fp, rEntry->FileName, L"rb");

					_fseeki64(fp, fOffset, FILE_BEGIN);
					int read = fread((uint8_t*)pBuffer + *nBytesRead, 1, toRead, fp);
					fclose(fp);

					if (read != toRead)
					{
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

uint64_t RamFS::FindEntryOffset(char * pFileName, uint64_t* pDataSize)
{
	for (int i = 0; i < this->NumEntries; ++i) {
		PS3FS_HEADER_ENTRY* entry = this->Entries[i].BinaryEntry;
		if (_stricmp(entry->Name, pFileName) == 0) {
			if (pDataSize)
				*pDataSize = entry->Size;
			return entry->Offset;
		}
	}
	return 0;
}

RamFS::Tracker *RamFS::CreateTracker()
{
	RamFS::Tracker* tracker = new RamFS::Tracker();
	tracker->Position = 0;
	tracker->FileSystem = this;
	return tracker;
}

void RamFS::LstGen(const char* pName) {
	char datNameBuf[64];
	char lstNameBuf[64];

	strcpy_s(datNameBuf, "data/");
	strcat_s(datNameBuf, pName);
	strcpy_s(lstNameBuf, datNameBuf);
	strcat_s(datNameBuf, ".dat");
	strcat_s(lstNameBuf, ".lst");

	FILE* inDataLstEn;
	int error;
	error = fopen_s(&inDataLstEn, lstNameBuf, "rb");
	// Skip if LST Already Exists
	if (error != 0) {
		PS3FS_HEADER header;
		FILE *inDataEn, *outDataLstEn;
		error = fopen_s(&inDataEn, datNameBuf, "rb");
		// Skip if DAT Doesn't Exist
		if (error == 0) {
			error = fopen_s(&outDataLstEn, lstNameBuf, "wb");

			fread(&header.Magic, sizeof(char), 8, inDataEn);
			fread(&header.NumEntries, sizeof(uint64_t), 1, inDataEn);

			for (int i = 0; i < header.NumEntries; i++) {
				fread(&header.Entries, sizeof(PS3FS_HEADER_ENTRY), 1, inDataEn);
				fwrite(header.Entries[0].Name, sizeof(char), strlen(header.Entries[0].Name), outDataLstEn);
				fwrite("\n", sizeof(char), 1, outDataLstEn);
			}
			fclose(outDataLstEn);
			fclose(inDataEn);
		}
	}
	else {
		fclose(inDataLstEn);
	}
}

RamFS* RamFS::Open(const char* pName) {
	wchar_t entryNameBuf[48];
	char datNameBuf[64];
	wchar_t dirNameBuf[64];
	wchar_t fileNameBuf[128];

	strcpy_s(datNameBuf, "data/");
	strcat_s(datNameBuf, pName);
	MultiByteToWideChar(CP_ACP, 0, datNameBuf, -1, dirNameBuf, 64);
	strcat_s(datNameBuf, ".dat");
	wcscat_s(dirNameBuf, L"/");

	FILE* inDatFile;
	int error = fopen_s(&inDatFile, datNameBuf, "rb");

	if (error != 0) {
		tcout << "Failed to Open Dat File: " << datNameBuf << "\n";
		return nullptr;
	}

	uint64_t nFiles = 0;
	fseek(inDatFile, 8, FILE_BEGIN);
	fread(&nFiles, 1, 8, inDatFile);

	RamFS* vfs = new RamFS();
	//vfs->OnFileRead = nullptr;
	vfs->NumEntries = nFiles;
	vfs->Entries = new RamFS::Entry[nFiles];
	vfs->BinaryHeader = PS3FS_HEADER::Prealloc(nFiles, &(vfs->BinaryHeaderSize));
	vfs->TotalSize = vfs->BinaryHeaderSize;

	for (int i = 0; i < nFiles; ++i) {
		PS3FS_HEADER_ENTRY* bEntry = &(vfs->BinaryHeader->Entries[i]);
		RamFS::Entry* rEntry = &(vfs->Entries[i]);

		rEntry->BinaryEntry = bEntry;

		PS3FS_HEADER_ENTRY datEntry;
		fread(&datEntry, 1, sizeof(PS3FS_HEADER_ENTRY), inDatFile);

		strcpy(bEntry->Name, datEntry.Name);

		MultiByteToWideChar(932, MB_PRECOMPOSED, bEntry->Name, -1, entryNameBuf, 48);

		wcscpy_s(fileNameBuf, dirNameBuf);
		wcscat_s(fileNameBuf, entryNameBuf);

		FILE* filePtr;
		int error = _wfopen_s(&filePtr, fileNameBuf, L"rb");
		if (!error) {
			fseek(filePtr, 0, FILE_END);
			int fileSize = ftell(filePtr);
			fclose(filePtr);


			wchar_t safeBuf[64];
			wcscpy_s(safeBuf, 64, fileNameBuf);
			for (int i = 0; i < wcslen(safeBuf); i++)
			{
				if (safeBuf[i] < ' ' || safeBuf[i] > '~')
					safeBuf[i] = '?';
			};
			tcout << L"Mounting File: " << safeBuf << L"\n";

			uint32_t fileNameLen = wcslen(fileNameBuf);
			rEntry->FileName = new wchar_t[fileNameLen + 1];
			wcscpy_s(rEntry->FileName, fileNameLen + 1, fileNameBuf);
			rEntry->BaseOffset = 0;
			rEntry->FileSize = fileSize;
			rEntry->Mounted = true;

			bEntry->Size = fileSize;
			bEntry->Offset = vfs->TotalSize;
			vfs->TotalSize += fileSize;
		}
		else {
			uint32_t fileNameLen = strlen(datNameBuf);
			rEntry->FileName = new wchar_t[fileNameLen + 1];
			MultiByteToWideChar(CP_ACP, 0, datNameBuf, -1, rEntry->FileName, fileNameLen);
			rEntry->FileName[fileNameLen] = 0;
			rEntry->BaseOffset = datEntry.Offset;
			rEntry->FileSize = datEntry.Size;
			rEntry->Mounted = false;

			bEntry->Size = datEntry.Size;
			bEntry->Offset = vfs->TotalSize;
			vfs->TotalSize += datEntry.Size;
		}
	}
	fclose(inDatFile);

	return vfs;
}