/*
 * Auto Generated Header File. Do Not Modify.
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
	0xE9, 0x6B, 0xA2, 0x12, 0x00
};

const unsigned int hDelta_01_base = 0x000A28D4;
const unsigned int hDelta_01_size = 5;
const unsigned char hDelta_01_data[] = {
	0xE9, 0xDD, 0x5A, 0x09, 0x00
};

const unsigned int hDelta_02_base = 0x000A341A;
const unsigned int hDelta_02_size = 1;
const unsigned char hDelta_02_data[] = {
	0x76
};

const unsigned int hDelta_03_base = 0x000A3491;
const unsigned int hDelta_03_size = 7;
const unsigned char hDelta_03_data[] = {
	0xE8, 0x39, 0x4F, 0x09, 0x00, 0xEB, 0xCE
};

const unsigned int hDelta_04_base = 0x000A4C24;
const unsigned int hDelta_04_size = 5;
const unsigned char hDelta_04_data[] = {
	0xE9, 0x8D, 0x37, 0x09, 0x00
};

const unsigned int hDelta_05_base = 0x000A55CD;
const unsigned int hDelta_05_size = 1;
const unsigned char hDelta_05_data[] = {
	0x76
};

const unsigned int hDelta_06_base = 0x000A5644;
const unsigned int hDelta_06_size = 7;
const unsigned char hDelta_06_data[] = {
	0xE8, 0x86, 0x2D, 0x09, 0x00, 0xEB, 0xCE
};

const unsigned int hDelta_07_base = 0x000A70B8;
const unsigned int hDelta_07_size = 5;
const unsigned char hDelta_07_data[] = {
	0xE9, 0xF9, 0x12, 0x09, 0x00
};

const unsigned int hDelta_08_base = 0x000A797B;
const unsigned int hDelta_08_size = 1;
const unsigned char hDelta_08_data[] = {
	0x76
};

const unsigned int hDelta_09_base = 0x000A79F2;
const unsigned int hDelta_09_size = 7;
const unsigned char hDelta_09_data[] = {
	0xE8, 0xD8, 0x09, 0x09, 0x00, 0xEB, 0xCE
};

const unsigned int hDelta_10_base = 0x000A9584;
const unsigned int hDelta_10_size = 5;
const unsigned char hDelta_10_data[] = {
	0xE9, 0x2D, 0xEE, 0x08, 0x00
};

const unsigned int hDelta_11_base = 0x000AA0D2;
const unsigned int hDelta_11_size = 1;
const unsigned char hDelta_11_data[] = {
	0x76
};

const unsigned int hDelta_12_base = 0x000AA149;
const unsigned int hDelta_12_size = 7;
const unsigned char hDelta_12_data[] = {
	0xE8, 0x81, 0xE2, 0x08, 0x00, 0xEB, 0xCE
};

const unsigned int hDelta_13_base = 0x000ABBD8;
const unsigned int hDelta_13_size = 5;
const unsigned char hDelta_13_data[] = {
	0xE9, 0xD9, 0xC7, 0x08, 0x00
};

const unsigned int hDelta_14_base = 0x000AC4A2;
const unsigned int hDelta_14_size = 1;
const unsigned char hDelta_14_data[] = {
	0x76
};

const unsigned int hDelta_15_base = 0x000AC519;
const unsigned int hDelta_15_size = 7;
const unsigned char hDelta_15_data[] = {
	0xE8, 0xB1, 0xBE, 0x08, 0x00, 0xEB, 0xCE
};

const unsigned int hDelta_16_base = 0x00138370;
const unsigned int hDelta_16_size = 117;
const unsigned char hDelta_16_data[] = {
	0x50, 0x60, 0x50, 0xE8, 0x00, 0x00, 0x00, 0x00, 
	0x58, 0x05, 0x3C, 0x10, 0x00, 0x00, 0xFF, 0x10, 
	0x89, 0x44, 0x24, 0x20, 0x61, 0x58, 0xC3, 0x00, 
	0x00, 0x60, 0x8B, 0x10, 0xFF, 0x70, 0x04, 0x33, 
	0xC0, 0x40, 0x8B, 0xC8, 0xE8, 0x07, 0xC7, 0xF2, 
	0xFF, 0x83, 0xC4, 0x04, 0x61, 0xC3, 0x00, 0x00, 
	0x50, 0x33, 0xC0, 0x48, 0xE8, 0xC7, 0xFF, 0xFF, 
	0xFF, 0x85, 0xC0, 0x74, 0x05, 0xE8, 0xD7, 0xFF, 
	0xFF, 0xFF, 0x58, 0xC3, 0x00, 0x00, 0x60, 0x33, 
	0xC0, 0x48, 0x48, 0xE8, 0xB0, 0xFF, 0xFF, 0xFF, 
	0x85, 0xC0, 0x74, 0x05, 0xE8, 0xC0, 0xFF, 0xFF, 
	0xFF, 0x61, 0xC2, 0x04, 0x00, 0x00, 0x00, 0x60, 
	0x33, 0xC0, 0x48, 0x48, 0x48, 0xE8, 0x96, 0xFF, 
	0xFF, 0xFF, 0x85, 0xC0, 0x74, 0x05, 0xE8, 0xA6, 
	0xFF, 0xFF, 0xFF, 0x61, 0xC3
};

const unsigned int hPatchCount = 17;
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
   { hDelta_09_base, hDelta_09_size, hDelta_09_data },
   { hDelta_10_base, hDelta_10_size, hDelta_10_data },
   { hDelta_11_base, hDelta_11_size, hDelta_11_data },
   { hDelta_12_base, hDelta_12_size, hDelta_12_data },
   { hDelta_13_base, hDelta_13_size, hDelta_13_data },
   { hDelta_14_base, hDelta_14_size, hDelta_14_data },
   { hDelta_15_base, hDelta_15_size, hDelta_15_data },
   { hDelta_16_base, hDelta_16_size, hDelta_16_data }
};
