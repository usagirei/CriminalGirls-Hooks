#pragma once
#include "PS3FS.h"
#include "stdafx.h"

#include <vector>
#include <iostream>


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
		bool Mounted = false;
		size_t Hash = 0;

		std::string FileName;
		uint64_t FileSize = 0;

		void(*FileProcessor)(const Entry&, char*) = nullptr;
		uint64_t ProcessedFileSize = 0;

		std::wstring SourceName;
		uint64_t SourceOffset = 0;
		PS3FS_HEADER_ENTRY* BinaryEntry = nullptr;

		explicit Entry(const std::string &str) : FileName(str), Hash(HashFunc(str)) {}

		static size_t HashFunc(const std::string &str) {
			std::hash<std::string> hashFn;
			return hashFn(str);
		}
	};

	typedef void(*FileReadCallback)(char* fileName);

	RamFS() {}

	~RamFS() {
		if (this->BinaryHeader) {
			this->BinaryHeader->Dealloc();
			this->BinaryHeader = nullptr;
		}

		for (size_t i = 0; i < this->Entries.size(); ++i)
			delete this->Entries[i];
		this->Entries.clear();
	}

	std::vector<FileReadCallback> OnFileRead;

	std::vector<Entry*> Entries;

	struct {
		size_t FileNameHash;
		uint32_t DataSize = 1024;
		char* DataBuffer = new char[1024];

		void Invalidate(const Entry &ent) {
			FileNameHash = ent.Hash;
			if (ent.ProcessedFileSize <= DataSize)
				return;
			DataSize = ent.ProcessedFileSize;
			delete[] DataBuffer;
			DataBuffer = new char[DataSize];
		}

		bool RequireInvalidate(const Entry &ent) {
			return FileNameHash != ent.Hash;
		}
	} ProcessingArea;

	uint64_t TotalSize = 0;
	uint64_t BinaryHeaderSize = 0;
	PS3FS_HEADER* BinaryHeader = nullptr;

	RamFS::Tracker* CreateTracker();

	RamFS::Entry* FindEntry(const std::string &rName);
	RamFS::Entry* CreateEntry(const std::string &rName);

	void MountDat(const std::string &pDatFileName);
	void MountDir(const std::string &pFolderName);
	void MountZip(const std::string &pZipFileName);

	void Build();
};

