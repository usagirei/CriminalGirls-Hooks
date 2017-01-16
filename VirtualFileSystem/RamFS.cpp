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
			tcout << "VFS::Read::Header\n";
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

						if (vfs->OnFileRead)
							(*vfs->OnFileRead)(bEntry->Name);

						//tcout << "VFS::Read::File::" << bEntry->Name << "\n";
						callback = true;
					}

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
						tcout << "FUCK";
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
	char lstNameBuf[64];
	wchar_t dirNameBuf[64];
	wchar_t fileNameBuf[128];

	strcpy_s(lstNameBuf, "data/");
	strcat_s(lstNameBuf, pName);
	MultiByteToWideChar(CP_ACP, 0, lstNameBuf, -1, dirNameBuf, 64);
	strcat_s(lstNameBuf, ".lst");
	wcscat_s(dirNameBuf, L"/");

	FILE* inDataLstEn;
	int error = fopen_s(&inDataLstEn, lstNameBuf, "rb");

	if (error != 0) {
		tcout << "Failed to Open Lst File: " << lstNameBuf;
		return nullptr;
	}

	int nFiles = -1;
	while (!feof(inDataLstEn)) {
		fgets((char*)entryNameBuf, 48, inDataLstEn);
		nFiles++;
	}
	fseek(inDataLstEn, 0, FILE_BEGIN);

	const int PAD = 0;
	RamFS* vfs = new RamFS();
	vfs->OnFileRead = nullptr;
	vfs->NumEntries = nFiles;
	vfs->Entries = new RamFS::Entry[nFiles];
	vfs->BinaryHeader = PS3FS_HEADER::Prealloc(nFiles, &(vfs->BinaryHeaderSize));
	vfs->TotalSize = vfs->BinaryHeaderSize;

	for (int i = 0; i < nFiles; ++i) {
		PS3FS_HEADER_ENTRY* bEntry = &(vfs->BinaryHeader->Entries[i]);
		RamFS::Entry* rEntry = &(vfs->Entries[i]);

		rEntry->BinaryEntry = bEntry;

		fgets(bEntry->Name, 48, inDataLstEn);
		size_t ln = strlen(bEntry->Name) - 1;
		if (*bEntry->Name && bEntry->Name[ln] == '\n')
			bEntry->Name[ln] = '\0';

		MultiByteToWideChar(932, MB_PRECOMPOSED, bEntry->Name, -1, entryNameBuf, 48);

		wcscpy_s(fileNameBuf, dirNameBuf);
		wcscat_s(fileNameBuf, entryNameBuf);

		FILE* filePtr;
		int error = _wfopen_s(&filePtr, fileNameBuf, L"rb");
		if (!error) {
			fseek(filePtr, 0, FILE_END);
			int fileSize = ftell(filePtr);
			int fileSizePad = (fileSize + PAD) & ~PAD;
			fclose(filePtr);

			uint32_t fileNameLen = wcslen(fileNameBuf);
			rEntry->FileName = new wchar_t[fileNameLen + 1];
			wcscpy_s(rEntry->FileName, fileNameLen + 1, fileNameBuf);
			rEntry->BaseOffset = 0;
			rEntry->FileSize = fileSizePad;

			bEntry->Size = fileSize;
			bEntry->Offset = vfs->TotalSize;
			vfs->TotalSize += fileSize;
		}
		else {
			rEntry->FileName = nullptr;
			char errBuf[100];
			strerror_s(errBuf, errno);
			tcout << _TEXT("Error Opening File #") << i << " - " << bEntry->Name << " - Reason: " << errBuf;

			RamFS::Close(vfs);

			vfs = nullptr;

			break;
		}
	}
	fclose(inDataLstEn);

	return vfs;
}