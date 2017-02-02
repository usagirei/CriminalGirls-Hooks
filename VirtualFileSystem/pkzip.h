#pragma once

#include <stdio.h>

const int ZIP_DIR_ENTRY_SIGNATURE = 0x02014B50;
const int ZIP_FILE_ENTRY_SIGNATURE = 0x04034B50;
const int ZIP_END_CD_SIGNATURE = 0x06054B50;

#pragma pack(push)
#pragma pack(1)

enum COMPTYPE : short {
	STORED = 0,
	SHRUNK = 1,
	REDUCED1 = 2,
	REDUCED2 = 3,
	REDUCED3 = 4,
	REDUCED4 = 5,
	IMPLODED = 6,
	TOKEN = 7,
	DEFLATE = 8,
	DEFLATE64 = 9
};


struct DOSDATETIME {
	static const short MASK_4 = (1 << 4) - 1;
	static const short MASK_5 = (1 << 5) - 1;
	static const short MASK_6 = (1 << 6) - 1;
	static const short MASK_7 = (1 << 7) - 1;

	short Time;
	short Date;

	int Second();
	int Minute();
	int Hour();
	int Day();
	int Month();
	int Year();
};


struct ZIPENDLOCATOR {
	int     Signature = ZIP_END_CD_SIGNATURE;
	short   DiskNumber = 0;
	short   StartDiskNumber = 0;
	short   EntriesOnDisk = 0;
	short   EntriesInDirectory = 0;
	int     DirectorySize = 0;
	int     DirectoryOffset = 0;
	short   CommentLength = 0;
	char    *Comment = nullptr;

	ZIPENDLOCATOR() {}
	~ZIPENDLOCATOR();
	explicit ZIPENDLOCATOR(FILE* f);

	ZIPENDLOCATOR(const ZIPENDLOCATOR &) = delete;
	void operator =(const ZIPENDLOCATOR &) = delete;
};

struct  ZIPDIRENTRY {
	int     Signature = ZIP_DIR_ENTRY_SIGNATURE;
	short   VersionMadeBy = 0;
	short   VersionToExtract = 0;
	short   Flags = 0;
	COMPTYPE Compression = COMPTYPE::STORED;
	DOSDATETIME  FileDateTime;
	int     Crc = 0;
	int     CompressedSize = 0;
	int     UncompressedSize = 0;
	short   FileNameLength = 0;
	short   ExtraFieldLength = 0;
	short   FileCommentLength = 0;
	short   DiskNumberStart = 0;
	short   InternalAttributes = 0;
	int     ExternalAttributes = 0;
	int     HeaderOffset = 0;
	char    *FileName = nullptr;
	char    *ExtraField = nullptr;
	char    *FileComment = nullptr;

	ZIPDIRENTRY() {}
	~ZIPDIRENTRY();
	explicit ZIPDIRENTRY(FILE* f);

	ZIPDIRENTRY(const ZIPDIRENTRY &) = delete;
	void operator =(const ZIPDIRENTRY &) = delete;
};

struct ZIPFILERECORD {
	int Signature = ZIP_FILE_ENTRY_SIGNATURE;
	short   Version = 0;
	short   Flags = 0;
	COMPTYPE Compression = COMPTYPE::STORED;
	DOSDATETIME  FileDateTime;
	int     Crc = 0;
	int     CompressedSize = 0;
	int     UncompressedSize = 0;
	short   FileNameLength = 0;
	short   ExtraFieldLength = 0;
	char    *FileName = nullptr;
	char    *ExtraField = nullptr;
	char    *Data = nullptr;

	ZIPFILERECORD() {}
	~ZIPFILERECORD();
	explicit ZIPFILERECORD(FILE* f, bool readData = true);

	ZIPFILERECORD(const ZIPFILERECORD &) = delete;
	void operator =(const ZIPFILERECORD &) = delete;
};

#pragma pack(pop)
