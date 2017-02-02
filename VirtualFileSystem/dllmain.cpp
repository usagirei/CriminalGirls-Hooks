// dllmain.cpp : Defines the entry point for the DLL application.
//#include <tchar.h>

#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#include "stdafx.h"
#include "VirtualFileSystem.h"

#include "patch.h"
#include "version.h"
#include "triggers.h"
#include "console.h"

typedef PDWORD(WINAPI *adDirect3DCreate9)(UINT sdkVersion);
#define HK_ENTRYPOINT_SYMBOL Direct3DCreate9
#define HK_ENTRYPOINT_DELEGATE adDirect3DCreate9
#define HK_ENTRYPOINT_MODULE _TEXT("d3d9.dll")

#define HK_ENTRYPOINT_SYMBOL_STR TOSTRING(HK_ENTRYPOINT_SYMBOL)
HK_ENTRYPOINT_DELEGATE ogEntryPointCall;

extern "C" PDWORD WINAPI HK_ENTRYPOINT_SYMBOL(UINT sdkVersion);
extern "C" BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

void TryLoadAudioTPK(char *fname);
void UpdateKnightState(char *fname);

bool ApplyPatches();
void Initialize();
void Deinitialize();

RamFS::Tracker* tpkTracker;

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			AllocConsole();
			freopen("CONOUT$", "w", stdout);

			TCHAR dllPath[260];
			const UINT dllPathSz = sizeof(dllPath) / sizeof(_TCHAR);
			const TCHAR* epModule = HK_ENTRYPOINT_MODULE;
			const UINT epModuleSz = sizeof(HK_ENTRYPOINT_MODULE) / sizeof(_TCHAR);

			GetSystemDirectory(dllPath, dllPathSz);
			_tcsncat_s(dllPath, dllPathSz, _TEXT("\\"), 2);
			_tcsncat_s(dllPath, dllPathSz, epModule, epModuleSz);
			HMODULE hEntry = LoadLibrary(dllPath);

			ogEntryPointCall = (HK_ENTRYPOINT_DELEGATE)GetProcAddress(hEntry, HK_ENTRYPOINT_SYMBOL_STR);

			Initialize();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			Deinitialize();
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

PDWORD WINAPI HK_ENTRYPOINT_SYMBOL(UINT sdkVersion) {

	if (!(sdkVersion & 0x80000000)) {
		return ogEntryPointCall(sdkVersion);
	}

	return ProcessTriggers(sdkVersion);
}

void Deinitialize() {
	delete VirtualFileSystem::DataEn;
	delete VirtualFileSystem::DataJp;
}

void Initialize() {

	tcout << VER_PRODUCTNAME_STR << " " << VER_VERSION_STRING << "\n";
	tcout << "Git Commit Hash: " << GIT_COMMIT_HASH << "\n";
	tcout << "Git Commit Date: " << GIT_COMMIT_DATE << "\n\n";

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	con::SetHandle(hConsole);
	tcout << "Applying Patches...\n";
	if (!ApplyPatches()) {
		tcout << con::fgCol<con::Red>;
		tcout << "Invalid File Checksum. Patches not Applied.\n";
		tcout << "Are you using a modified executable?\n";
	}
	else {
		tcout << con::fgCol<con::Green>;
		tcout << "Patches Applied\n";
	}
	tcout << con::fgCol<con::Gray>;

	srand(time(NULL));

	tpkTracker = nullptr;

	tcout << "Initializing Virtual File System...\n\n";
	VirtualFileSystem::Initialize();

	//TODO: Find Way to detect which locale is loaded

	AudioState.TpkDataA = new char[4 * 1024 * 1024];
	AudioState.TpkDataB = new char[4 * 1024 * 1024];

	if (VirtualFileSystem::DataEn) {
		VirtualFileSystem::DataEn->OnFileRead.push_back(&TryLoadAudioTPK);
		VirtualFileSystem::DataEn->OnFileRead.push_back(&UpdateKnightState);
		tpkTracker = VirtualFileSystem::DataEn->CreateTracker();
	}
	else {
		tcout << con::fgCol<con::Red>;
		tcout << "Couldn't Initialize En Virtual File System!\n";
		tcout << con::fgCol<con::Gray>;
	}

	if (VirtualFileSystem::DataJp) {
		VirtualFileSystem::DataJp->OnFileRead.push_back(&TryLoadAudioTPK);
		VirtualFileSystem::DataJp->OnFileRead.push_back(&UpdateKnightState);
		//tpkTracker = VirtualFileSystem::DataJp->CreateTracker();
	}
	else {
		tcout << con::fgCol<con::Red>;
		tcout << "Couldn't Initialize JP Virtual File System!\n";
		tcout << con::fgCol<con::Gray>;
	}

}

bool ApplyPatches() {
	HMODULE module = GetModuleHandle(0);
	TCHAR filePath[260];
	GetModuleFileName(module, filePath, 260);

	FILE* file;
	int error = _tfopen_s(&file, filePath, _TEXT("rb"));
	if (error)
		return false;

	uint32_t crc32 = 0;
	unsigned char *crcBuf = new  unsigned char[4096];
	while (!feof(file)) {
		int read = fread(crcBuf, 1, 4096, file);
		crc32 = hPatchCRC32(crc32, crcBuf, read);
	}
	fclose(file);
	delete[] crcBuf;

	if (crc32 != hPatchChecksum)
		return false;

	uint8_t *base = (uint8_t*)module;
	for (int i = 0; i < hPatchCount; ++i) {
		const uint32_t offset = hPatches[i].Address;
		const uint32_t size = hPatches[i].Size;
		const uint8_t* data = hPatches[i].Data;
		uint8_t* addr = base + offset;

		DWORD accessProtectionValue, accessProtec;
		int vProtect = VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &accessProtectionValue);
		memcpy(addr, data, size);
		vProtect = VirtualProtect(addr, size, accessProtectionValue, &accessProtec);
	}
	return true;
}

void TryLoadAudioTPK(char* fName) {
	int len = strlen(fName);
	if (len < 5)
		return;

	if (!tpkTracker) {
		return;
	}

	char buf[32];
	strcpy_s(buf, fName + (len - 4));
	int cmp = _stricmp(buf, "json");
	if (cmp != 0)
		return;

	HMODULE base = GetModuleHandle(nullptr);
	if (_DR1(base, 0x1C9554))
	{
		GameState.Option = _DR5(base, 0x1C9554, 0x1C, 0x54C, 0x5E8, 0x1E7C);
		GameState.MaxTries = _DR2(base, 0x1C9560, 0xF0);
	}
	else
	{
		// Probably Opened Scene Viewer
		GameState.Option = 0;
		GameState.MaxTries = -1;
	}

	char ibuf[] = "x";
	strncpy_s(buf, fName, len - 11);
	len = strlen(buf);

	ibuf[0] = *(buf + len - 1);
	GameState.Level = atoi(ibuf) - 1;

	ibuf[0] = *(buf + len - 3);
	GameState.MiniGame = atoi(ibuf) - 1;

	strncpy_s(GameState.Girl, fName, 3);

	if (GameState.MiniGame == 4)
		GameState.Level = 0;

	//GameState.Level = min(GameState.Level, 2);

	AudioState.Delay = 180; // Initial Delay

	DWORD tpkRd;
	uint64_t tpkSz;
	uint64_t tpkPos;
	uint64_t nFiles, nFilesPad;
	struct TPKENTRY {
		char Name[40];
		uint32_t Size;
		uint32_t Offset;
	} *entries;

	bool knighted = false;
	for (int i = 0; i < 9; i++)
	{
		if (_stricmp(GameState.Girl, GameState.KnightStatus[i].Girl) == 0) {
			knighted = GameState.KnightStatus[i].Status;
			break;
		}
	}

	int audioSet = knighted 
		? 2 
		: (GameState.Level >= 2 
			? 1 
			: 0);

	// Breathing - Banter TPK
	sprintf_s(buf, "pan_react_%s%02d_%d.tpk", GameState.Girl, GameState.MiniGame, (GameState.MiniGame == 4 ? 0 : audioSet));
	if (strcmp(fName, AudioState.TpkDataAName) != 0) {
		strcpy_s(AudioState.TpkDataAName, fName);
		tcout << "Loading TPK A " << buf << "\n";

		auto entry = tpkTracker->FileSystem->FindEntry(buf);

		tpkPos = entry->BinaryEntry->Offset;
		tpkSz = entry->BinaryEntry->Size;

		tpkTracker->Seek(tpkPos, FILE_BEGIN);
		tpkTracker->Read(AudioState.TpkDataA, tpkSz, &tpkRd);

		nFiles = *(uint64_t*)(AudioState.TpkDataA + 0x08);
		nFilesPad = (nFiles + 1) & ~1;

		entries = (TPKENTRY*)(AudioState.TpkDataA + 16 + (2 * nFilesPad));

		if (AudioState.OggDataA)
			delete[] AudioState.OggDataA;

		AudioState.OggDataA = new OggData[nFiles];
		AudioState.NumOggFilesA = nFiles;

		for (int i = 0; i < nFiles;++i) {
			AudioState.OggDataA[i].Data = (uint8_t*)(AudioState.TpkDataA + entries[i].Offset);
			AudioState.OggDataA[i].Size = entries[i].Size;
		}
	}

	// Reactions TPK
	sprintf_s(buf, "pan_react_%s_%d.tpk", GameState.Girl, (3 * GameState.Option) + audioSet);
	if (strcmp(fName, AudioState.TpkDataBName) != 0) {
		strcpy_s(AudioState.TpkDataBName, fName);
		tcout << "Loading TPK B " << buf << "\n";

		auto entry = tpkTracker->FileSystem->FindEntry(buf);
		
		tpkPos = entry->BinaryEntry->Offset;
		tpkSz = entry->BinaryEntry->Size;

		tpkTracker->Seek(tpkPos, FILE_BEGIN);
		tpkTracker->Read(AudioState.TpkDataB, tpkSz, &tpkRd);

		nFiles = *(uint64_t*)(AudioState.TpkDataB + 0x08);
		nFilesPad = (nFiles + 1) & ~1;

		entries = (TPKENTRY*)(AudioState.TpkDataB + 16 + (2 * nFilesPad));

		if (AudioState.OggDataB)
			delete[] AudioState.OggDataB;

		AudioState.OggDataB = new OggData[nFiles];
		AudioState.NumOggFilesB = (uint32_t) nFiles;

		for (int i = 0; i < nFiles;++i) {
			AudioState.OggDataB[i].Data = (uint8_t*)(AudioState.TpkDataB + entries[i].Offset);
			AudioState.OggDataB[i].Size = entries[i].Size;
		}
	}
}

void UpdateKnightState(char* fName) {
	if (_strnicmp(fName, "if_camp_", 8) != 0)
		return;
	if (_strnicmp(fName, "if_camp_res", 11) == 0)
		return;

	char girlName[4];
	char atoiBuf[] = "x";

	strncpy_s(girlName, fName + 8, 3);
	strncpy_s(atoiBuf, fName + 12, 1);
	bool knighted = atoi(atoiBuf) > 0;

	for (int i = 0; i < 9; i++) {
		auto *entry = &GameState.KnightStatus[i];
		bool empty = entry->Girl[0] == 0;
		if (empty || (_stricmp(entry->Girl, girlName) == 0)) {
			if (empty)
				strcpy_s(entry->Girl, girlName);
			entry->Status = knighted;
			break;
		}
	}
}




