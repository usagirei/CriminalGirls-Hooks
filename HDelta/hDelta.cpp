// HDelta.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <inttypes.h>
#include <vector>
#include <fstream>
#include <iomanip>
#include <windows.h>

#include "stdafx.h"

#define DEC(x)   std::dec << (x)
#define DEC2(x)  std::setfill('0') << std::setw(2) << std::dec << (x)
#define HEX2(x) "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (uint16_t)((unsigned char)(x) & 0xFF)
#define HEX8(x) "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << (uint32_t)(x)

struct PATCH_DEF {
	uint32_t Addr;
	uint32_t Size;
	uint8_t* Data;

	PATCH_DEF(uint32_t addr, uint32_t sz, uint8_t* data) : Addr(addr), Size(sz), Data(data) {

	}
};

unsigned char* LoadSection(TCHAR* exeName, char* secName, uint32_t *codeSize, uint32_t *vAddr) {
	IMAGE_DOS_HEADER dosHeader;
	IMAGE_NT_HEADERS ntHeaders;
	IMAGE_SECTION_HEADER section;

	std::ifstream exeStream(exeName, std::ios::binary);
	exeStream.read((char*)&dosHeader, sizeof(IMAGE_DOS_HEADER));

	exeStream.seekg(dosHeader.e_lfanew, std::ios::beg);
	exeStream.read((char*)&ntHeaders, sizeof(IMAGE_NT_HEADERS));

	for (int i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++) {
		exeStream.read((char*)&section, sizeof(IMAGE_SECTION_HEADER));
		if (_strcmpi((char*)section.Name, secName) == 0) {
			*codeSize = section.SizeOfRawData;
			*vAddr = section.VirtualAddress;

			unsigned char* code = new unsigned char[section.SizeOfRawData];
			exeStream.seekg(section.PointerToRawData, std::ios::beg);
			exeStream.read((char*)code, section.SizeOfRawData);
			return code;
		}
	}
	return nullptr;
}


int _tmain(int argc, TCHAR **argv)
{
	TCHAR *inputName = argv[1];
	TCHAR *cleanName = argv[2];
	TCHAR *patchName = argv[3];

	uint32_t cleanSize, inputSize, vAddr;
	unsigned char* inputCode = LoadSection(inputName, ".text", &inputSize, &vAddr);
	unsigned char* cleanCode = LoadSection(cleanName, ".text", &cleanSize, &vAddr);

	std::ofstream patch(patchName);

	int patchNum = 0;

	std::vector<uint8_t>* curPatch = new std::vector<uint8_t>();
	std::vector<uint8_t>* curData = new std::vector<uint8_t>();
	std::vector<PATCH_DEF>* patches = new std::vector<PATCH_DEF>();

	for (unsigned int i = 0; i < cleanSize; ++i) {
		char in = inputCode[i];
		char cl = cleanCode[i];

		//int pos = input.tellg();

		if (in != cl) {
			curData->clear();
			curPatch->clear();

			uint32_t pos = i;

			const int LOOKAHEAD = 8;
			int scan = LOOKAHEAD;
			while (true) {
				curPatch->push_back(in ^ cl);
				curData->push_back(cl);

				++i;
				in = inputCode[i];
				cl = cleanCode[i];

				scan = (in == cl)
					? scan - 1
					: LOOKAHEAD;

				if (scan == 0)
					break;
			}

			while (curPatch->back() == 0) {
				curPatch->pop_back();
			}

			for (unsigned int j = 0; j < curPatch->size(); ++j) {
				uint8_t value = (curPatch->at(j)) ^ (curData->at(j));
				curPatch->at(j) = value;
			}

			uint32_t addr = pos + vAddr;
			uint32_t size = curPatch->size();
			uint8_t* data = new uint8_t[size];
			std::copy(curPatch->begin(), curPatch->end(), data);
			patches->push_back(PATCH_DEF(addr, size, data));
		}
	}

	uint32_t remSize = inputSize - cleanSize - 1;
	unsigned char* remCode = inputCode + cleanSize;
	for (int i = remSize; i >= 0; --i) {
		if (remCode[i] != 0) {
			break;
		}
		remSize = i;
	}
	if (remSize > 0) {
		patches->push_back(PATCH_DEF(cleanSize + vAddr, remSize, remCode));
	}

	char *inputBuf = new  char[32];
	char *cleanBuf = new  char[32];
	char *timeBuf = new  char[32];
	char *dateBuf = new  char[32];

	GetDateFormatA(LOCALE_INVARIANT, 0, nullptr, "MMM dd, yyyy", dateBuf, 32);
	GetTimeFormatA(LOCALE_INVARIANT, 0, nullptr, "HH:mm:ss", timeBuf, 32);
	TCHAR* inputName2 = _tcsrchr(inputName, '\\') + 1;
	TCHAR* cleanName2 = _tcsrchr(cleanName, '\\') + 1;
	WideCharToMultiByte(CP_ACP, 0, inputName2, -1, inputBuf, 32, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, cleanName2, -1, cleanBuf, 32, nullptr, nullptr);

	patch << "/*\n";
	patch << " * Auto Generated Header File. Do Not Modify.\n";
	// patch << " * Generated On " << dateBuf << " - " << timeBuf << "\n";
	patch << " * Target Data: " << inputBuf << "\n";
	patch << " * Reference Data: " << cleanBuf << "\n";
	patch << " */\n";

	delete[] inputBuf;
	delete[] cleanBuf;
	delete[] timeBuf;
	delete[] dateBuf;

	patch << "#pragma once\n\n";
	patch << "typedef struct HPATCH_DATA {\n";
	patch << "   const unsigned int Address;\n";
	patch << "   const unsigned int Size;\n";
	patch << "   const unsigned char* Data;\n";
	patch << "   HPATCH_DATA(const unsigned int _a, const unsigned int _b, const unsigned char *_c) : Address(_a), Size(_b), Data(_c) {};\n";
	patch << "} HPATCH_DATA;\n";
	patch << "\n";

	for (unsigned int i = 0; i < patches->size(); ++i) {
		PATCH_DEF p = patches->at(i);

		patch << "const unsigned int hDelta_" << DEC2(i) << "_base = " << HEX8(p.Addr) << ";\n";
		patch << "const unsigned int hDelta_" << DEC2(i) << "_size = " << DEC(p.Size) << ";\n";
		patch << "const unsigned char hDelta_" << DEC2(i) << "_data[] = {\n\t";
		for (unsigned int j = 0; j < p.Size; ++j) {
			uint8_t value = p.Data[j];
			if (j == p.Size - 1)
				patch << HEX2(value);
			else
				patch << HEX2(value) << ", ";

			if ((j + 1) % 8 == 0)
				patch << "\n\t";
		}
		patch << "\n};\n\n";
	}

	patch << "const unsigned int hPatchCount = " << DEC(patches->size()) << ";\n";
	patch << "const HPATCH_DATA hPatches[] = {\n";
	for (unsigned int i = 0; i < patches->size(); ++i) {
		patch << "   { hDelta_" << DEC2(i) << "_base, hDelta_" << DEC2(i) << "_size, hDelta_" << DEC2(i) << "_data }";
		if (i == patches->size() - 1)
			patch << "\n";
		else
			patch << ",\n";
	}
	patch << "};\n";

	delete cleanCode;
	delete inputCode;

	delete curData;
	delete curPatch;
	for (unsigned int i = 0; i < patches->size(); i++)
		delete patches->at(i).Data;
	delete patches;
};

