// dllmain.cpp : Defines the entry point for the DLL application.
//#include <tchar.h>

#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#include "stdafx.h"
#include "VirtualFileSystem.h"

typedef PDWORD(WINAPI *adDirect3DCreate9)(UINT sdkVersion);
#define HK_ENTRYPOINT_SYMBOL Direct3DCreate9
#define HK_ENTRYPOINT_DELEGATE adDirect3DCreate9
#define HK_ENTRYPOINT_MODULE _TEXT("d3d9.dll")

#define HK_ENTRYPOINT_SYMBOL_STR TOSTRING(HK_ENTRYPOINT_SYMBOL)
HK_ENTRYPOINT_DELEGATE ogEntryPointCall;

struct OggData{
	uint8_t* Data;
	uint32_t Size;
};

char loadedTpk[32] = { 0 };
char* tpkData;
OggData* oggData;
int nOggFiles;
RamFS::Tracker* tpkTracker;

extern "C" PDWORD WINAPI HK_ENTRYPOINT_SYMBOL(UINT sdkVersion) {

	if (sdkVersion == 0xFFFFFFFF) {
		int nth = rand() % nOggFiles;
		tcout << "Playing Random OGG File [" << nth << "]\n";
		return (PDWORD)&oggData[nth];
	}
	else {
		return ogEntryPointCall(sdkVersion);
	}
}

void DetectTargetTPK(char* fName) {
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

	char nBuf[4];
	char ibuf[] = "x";
	strncpy_s(buf, fName, len - 11);
	len = strlen(buf);

	ibuf[0] = *(buf + len - 1);
	int lvl = atoi(ibuf) - 1;

	ibuf[0] = *(buf + len - 3);
	int minigame = atoi(ibuf) - 1;

	strncpy_s(nBuf, fName, 3);

	if (minigame == 4)
		lvl = 0;

	lvl = min(lvl, 2);

	sprintf_s(buf, "pan_react_%s%02d_%d.tpk", nBuf, minigame, lvl);
	if (strcmp(buf, loadedTpk) == 0)
		return;

	strcpy_s(loadedTpk, buf);

	tcout << "Loading TPK " << buf << "\n";

	DWORD tpkRd;
	uint64_t tpkSz;
	uint64_t tpkPos = tpkTracker->FileSystem->FindEntryOffset(buf, &tpkSz);
	tpkTracker->Seek(tpkPos, FILE_BEGIN);
	tpkTracker->Read(tpkData, tpkSz, &tpkRd);

	uint64_t nFiles = *(uint64_t*)(tpkData + 0x08);
	uint64_t nFilesPad = (nFiles + 1) & ~1;

	struct TPKENTRY {
		char Name[40];
		uint32_t Size;
		uint32_t Offset;
	}*entries = (TPKENTRY*)(tpkData + 16 + (2 * nFilesPad));

	if (oggData)
		delete[] oggData;

	oggData = new OggData[nFiles];
	nOggFiles = nFiles;

	for (int i = 0; i < nFiles;++i) {
		oggData[i].Data = (uint8_t*)(tpkData + entries[i].Offset);
		oggData[i].Size = entries[i].Size;
	}
}

extern "C" BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			srand(time(nullptr));

			tpkTracker = nullptr;
			oggData = nullptr;
			nOggFiles = 0;
			tpkData = nullptr;

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

			VirtualFileSystem::Initialize();

			if (VirtualFileSystem::DataEn) {
				VirtualFileSystem::DataEn->OnFileRead = &DetectTargetTPK;
				tpkTracker = VirtualFileSystem::DataEn->CreateTracker();
				tpkData = new char[4 * 1024 * 1024];
			}
			else {
				tcout << "Couldn't Initialize Virtual File System";
			}

			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}




