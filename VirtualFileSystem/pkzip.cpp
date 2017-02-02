#include "pkzip.h"


ZIPENDLOCATOR::ZIPENDLOCATOR(FILE *f) {
	fread(this, sizeof(ZIPENDLOCATOR) - (1 * sizeof(char*)), 1, f); // 1 Pointer
	Comment = new char[CommentLength + 1]{ 0 };
	fread(Comment, sizeof(char), CommentLength, f);
}


ZIPDIRENTRY::ZIPDIRENTRY(FILE* f) {
	fread(this, sizeof(ZIPDIRENTRY) - (3 * sizeof(char*)), 1, f); // 3 Pointers

	FileName = new char[FileNameLength + 1]{ 0 };
	fread(FileName, sizeof(char), FileNameLength, f);

	ExtraField = new char[ExtraFieldLength + 1]{ 0 };
	fread(ExtraField, sizeof(char), ExtraFieldLength, f);

	FileComment = new char[FileCommentLength + 1]{ 0 };
	fread(FileComment, sizeof(char), FileCommentLength, f);
}


ZIPFILERECORD::ZIPFILERECORD(FILE * f, bool readData)
{
	fread(this, sizeof(ZIPFILERECORD) - (3 * sizeof(char*)), 1, f); // 3 Pointers

	FileName = new char[FileNameLength + 1]{ 0 };
	fread(FileName, sizeof(char), FileNameLength, f);

	ExtraField = new char[ExtraFieldLength + 1]{ 0 };
	fread(ExtraField, sizeof(char), ExtraFieldLength, f);

	if (readData) {
		Data = new char[CompressedSize] { 0 };
		fread(Data, sizeof(char), CompressedSize, f);
	}
	else {
		fseek(f, CompressedSize, SEEK_CUR);
	}
}

ZIPENDLOCATOR::~ZIPENDLOCATOR() {
	if (Comment) delete[] Comment;
}

ZIPDIRENTRY::~ZIPDIRENTRY() {
	if (FileName) delete[] FileName;
	if (ExtraField) delete[] ExtraField;
	if (FileComment) delete[] FileComment;
}

ZIPFILERECORD::~ZIPFILERECORD()
{
	if (FileName) delete[] FileName;
	if (ExtraField) delete[] ExtraField;
	if (Data) delete[] Data;
}


int DOSDATETIME::Second() {
	return (Time >> 0 & MASK_5) << 1;
}

int DOSDATETIME::Minute() {
	return (Time >> 5 & MASK_6);
}

int DOSDATETIME::Hour() {
	return (Time >> 11 & MASK_5);
}

int DOSDATETIME::Day() {
	return (Date >> 0 & MASK_5);
}

int DOSDATETIME::Month() {
	return (Date >> 5 & MASK_4);
}

int DOSDATETIME::Year() {
	return 1980 + (Date >> 9 & MASK_7);
}