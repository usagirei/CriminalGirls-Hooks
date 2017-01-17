/*
 * Auto Generated Header File. Do Not Modify.
 * Generated On Jan 17, 2017 - 18:52:56
 * Target Data: steam_criminal_mod.exe
 * Reference Data: steam_criminal_orig.exe
 */
#pragma once

typedef struct HPATCH_DATA {
   const unsigned int Address;
   const unsigned int Size;
   const unsigned char* Data;
   HPATCH_DATA(const unsigned int _a, const unsigned int _b, const unsigned char *_c) : Address(_a), Size(_b), Data(_c) {};
} HPATCH_DATA;

const unsigned int hDelta_00_base = 0x0000E130;
const unsigned int hDelta_00_size = 5;
const unsigned char hDelta_00_data[] = {
	0xE9, 0x2F, 0xA2, 0x12, 0x00
};

const unsigned int hDelta_01_base = 0x0009E598;
const unsigned int hDelta_01_size = 6;
const unsigned char hDelta_01_data[] = {
	0xE8, 0x16, 0x0B, 0x00, 0x00, 0x90
};

const unsigned int hDelta_02_base = 0x0009F0B3;
const unsigned int hDelta_02_size = 12;
const unsigned char hDelta_02_data[] = {
	0x5E, 0x81, 0xC6, 0xCF, 0x92, 0x12, 0x00, 0xE9, 
	0xC4, 0x01, 0x00, 0x00
};

const unsigned int hDelta_03_base = 0x0009F283;
const unsigned int hDelta_03_size = 11;
const unsigned char hDelta_03_data[] = {
	0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x95, 
	0x00, 0x00, 0x00
};

const unsigned int hDelta_04_base = 0x0009F323;
const unsigned int hDelta_04_size = 11;
const unsigned char hDelta_04_data[] = {
	0x8B, 0xB7, 0xF8, 0x00, 0x00, 0x00, 0xE9, 0x70, 
	0xF2, 0xFF, 0xFF
};

const unsigned int hDelta_05_base = 0x000ABB78;
const unsigned int hDelta_05_size = 28;
const unsigned char hDelta_05_data[] = {
	0x8D, 0x76, 0x04, 0x8B, 0x0E, 0xE8, 0xEE, 0xF6, 
	0x04, 0x00, 0xE8, 0xFC, 0x01, 0x00, 0x00, 0x7D, 
	0x0B, 0x8B, 0x0E, 0xE8, 0x10, 0xFF, 0x04, 0x00, 
	0x90, 0x90, 0x90, 0x90
};

const unsigned int hDelta_06_base = 0x000ABD83;
const unsigned int hDelta_06_size = 12;
const unsigned char hDelta_06_data[] = {
	0x59, 0x81, 0xC1, 0xE5, 0xBC, 0x11, 0x00, 0xE9, 
	0xD2, 0x0A, 0x00, 0x00
};

const unsigned int hDelta_07_base = 0x000AC861;
const unsigned int hDelta_07_size = 10;
const unsigned char hDelta_07_data[] = {
	0x8B, 0x09, 0x83, 0xE9, 0x06, 0xE9, 0x1C, 0xF3, 
	0xFF, 0xFF
};

const unsigned int hDelta_08_base = 0x000C7101;
const unsigned int hDelta_08_size = 10;
const unsigned char hDelta_08_data[] = {
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
	0x90, 0x90
};

const unsigned int hDelta_09_base = 0x00138364;
const unsigned int hDelta_09_size = 42;
const unsigned char hDelta_09_data[] = {
	0x50, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0xE8, 
	0x00, 0x00, 0x00, 0x00, 0x58, 0x05, 0x40, 0x10, 
	0x00, 0x00, 0xFF, 0x10, 0x8B, 0x10, 0xFF, 0x70, 
	0x04, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x8B, 0xC8, 
	0xE8, 0x17, 0xC7, 0xF2, 0xFF, 0x83, 0xC4, 0x04, 
	0x58, 0xC3
};

const unsigned int hPatchCount = 10;
const HPATCH_DATA hPatches[] = {
   { hDelta_00_base, hDelta_00_size, hDelta_00_data },
   { hDelta_01_base, hDelta_01_size, hDelta_01_data },
   { hDelta_02_base, hDelta_02_size, hDelta_02_data },
   { hDelta_03_base, hDelta_03_size, hDelta_03_data },
   { hDelta_04_base, hDelta_04_size, hDelta_04_data },
   { hDelta_05_base, hDelta_05_size, hDelta_05_data },
   { hDelta_06_base, hDelta_06_size, hDelta_06_data },
   { hDelta_07_base, hDelta_07_size, hDelta_07_data },
   { hDelta_08_base, hDelta_08_size, hDelta_08_data },
   { hDelta_09_base, hDelta_09_size, hDelta_09_data }
};
