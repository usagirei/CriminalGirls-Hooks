#pragma once
#include <string>
#include <inttypes.h>

struct PS3FS_HEADER_ENTRY {
	char Name[48];
	uint64_t Size;
	uint64_t Offset;
};

struct PS3FS_HEADER {
	unsigned char Magic[8];
	uint64_t NumEntries;
	PS3FS_HEADER_ENTRY Entries[1];

	static const uint64_t MagicData = 0x31565F5346335350;

	static PS3FS_HEADER* Prealloc(uint64_t nEntries, uint64_t* dataSize) {
		uint64_t size = sizeof(Magic) + sizeof(NumEntries) + (sizeof(PS3FS_HEADER_ENTRY) * nEntries);
		char* prealloc = new char[size] {0};
		*dataSize = size;
		return ::new (prealloc) PS3FS_HEADER(nEntries);
	}

	PS3FS_HEADER() : NumEntries(1), Entries()
	{
		*(uint64_t*)Magic = MagicData;
	}

	void Dealloc() {
		delete[](char*) this;
	}

	PS3FS_HEADER(const PS3FS_HEADER &) = delete;
	void operator =(const PS3FS_HEADER &) = delete;

private:
	explicit PS3FS_HEADER(uint64_t nEntries) : NumEntries(nEntries)
	{
		memcpy((void*)Magic, &MagicData, 8);
	}
};
