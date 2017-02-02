#pragma once
#include "PS3FS.h"
#include "stdafx.h"
#include <vector>


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
		std::string FileName;
		uint64_t FileSize;
		std::wstring SourceName;
		uint64_t SourceOffset;
		PS3FS_HEADER_ENTRY* BinaryEntry;
	};

	typedef void(*FileReadCallback)(char* fileName);

	RamFS() {
		BinaryHeader = nullptr;
		TotalSize = 0;
		BinaryHeaderSize = 0;
	}

	~RamFS() {
		if (this->BinaryHeader) {
			this->BinaryHeader->Dealloc();
			this->BinaryHeader = nullptr;
		}

		for (int i = 0; i < this->Entries.size(); ++i)
			delete this->Entries[i];
		this->Entries.clear();
	}

	std::vector<FileReadCallback> OnFileRead;

	std::vector<Entry*> Entries;

	uint64_t TotalSize;
	uint64_t BinaryHeaderSize;
	PS3FS_HEADER* BinaryHeader;

	RamFS::Tracker* CreateTracker();

	RamFS::Entry* FindEntry(const std::string &rName);
	RamFS::Entry* CreateEntry(const std::string &rName);

	void MountDat(const std::string &pDatFileName);
	void MountDir(const std::string &pFolderName);
	void MountZip(const std::string &pZipFileName);

	void Build();
};

