#pragma once
#include "PS3FS.h"
#include "stdafx.h"


class RamFS {
public:
	class Tracker {
	public:
		RamFS* FileSystem;
		uint64_t Position;

		void Read(PVOID pBuffer, DWORD nBytesToRead, PDWORD nBytesRead);
		uint64_t Seek(DWORD nOffset, DWORD eMoveMethod);
	};

	class Entry {
	public:
		bool Mounted;
		wchar_t* FileName;
		uint64_t FileSize;
		uint64_t BaseOffset;
		PS3FS_HEADER_ENTRY* BinaryEntry;
	};

	typedef void (*FileReadCallback)(char* fileName);

	// TODO: Proper Invocation List
	FileReadCallback OnFileRead;

	Entry* Entries;
	uint64_t NumEntries;
	uint64_t BinaryHeaderSize;
	uint64_t TotalSize;
	PS3FS_HEADER* BinaryHeader;

	RamFS::Tracker* CreateTracker();
	uint64_t FindEntryOffset(char * pFileName, uint64_t* pDataSize);

	static void LstGen(const char* pDataFileName);
	static RamFS* Open(const char* pDataFileName);
	static void Close(RamFS* pFileSystem);
};

