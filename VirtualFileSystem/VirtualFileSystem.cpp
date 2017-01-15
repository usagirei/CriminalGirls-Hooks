// VirtualFileSystem.cpp : Defines the exported functions for the DLL application.
//

#include <iostream>
#include <sstream>
#include <cwchar>
#include <map>

#include "stdafx.h"
#include "iathook.h"
#include "VirtualFileSystem.h"

#include "RamVFS.h"

#define HK_MSVCRUNTIME_MODULE _TEXT("\\msvcr120.dll")
#define HK_KERNEL32_MODULE "kernel32.dll"

adCreateFileW VirtualFileSystem::ogCreateFileW;
adCreateFileA VirtualFileSystem::ogCreateFileA;
adSetFilePointer VirtualFileSystem::ogSetFilePointer;
adSetFilePointerEx VirtualFileSystem::ogSetFilePointerEx;
adReadFile VirtualFileSystem::ogReadFile;
adCloseHandle VirtualFileSystem::ogCloseHandle;

typedef std::map<HANDLE, RamFS::Tracker*> RamVFS_ViewMap;

RamVFS_ViewMap* openViews;
RamFS* dataEnVFS;
RamFS* dataJpVFS;

void VirtualFileSystem::Initialize()
{
	openViews = new RamVFS_ViewMap();
	dataEnVFS = nullptr;
	dataJpVFS = nullptr;

	TCHAR dllPath[260];
	GetSystemDirectory(dllPath, sizeof(dllPath) / sizeof(_TCHAR));
	_tcsncat_s(dllPath, sizeof(dllPath) / sizeof(_TCHAR), HK_MSVCRUNTIME_MODULE, sizeof(HK_MSVCRUNTIME_MODULE) / sizeof(_TCHAR));
	HMODULE msvcr = LoadLibrary(dllPath);

	RamFS::LstGen("data_en");
	//VFSLstGen("data_jp");

	dataEnVFS = RamFS::Open("data_en");

	//RAMVFS_VIEW* view = VFSCreateView(dataEnVFS);
	//uint64_t toWrite = dataEnVFS->TotalSize;
	//FILE* fp = fopen("data/data.dat", "wb");
	//char buf[64*1024];
	//DWORD read;
	//while (toWrite > 0) {
	   // VFSRead(view, buf, sizeof(buf), &read);
	   // fwrite(buf, 1, read, fp);
	   // toWrite -= read;
	//}
	//fclose(fp);

   //dataJpVFS = VFSOpen("data_jp");

	IATHook32<adCreateFileW>(msvcr, HK_KERNEL32_MODULE, "CreateFileW", VirtualFileSystem::CreateFileW, &VirtualFileSystem::ogCreateFileW);
	IATHook32<adCreateFileA>(msvcr, HK_KERNEL32_MODULE, "CreateFileA", VirtualFileSystem::CreateFileA, &VirtualFileSystem::ogCreateFileA);
	IATHook32<adSetFilePointer>(msvcr, HK_KERNEL32_MODULE, "SetFilePointer", VirtualFileSystem::SetFilePointer, &VirtualFileSystem::ogSetFilePointer);
	IATHook32<adSetFilePointerEx>(msvcr, HK_KERNEL32_MODULE, "SetFilePointerEx", VirtualFileSystem::SetFilePointerEx, &VirtualFileSystem::ogSetFilePointerEx);
	IATHook32<adCloseHandle>(msvcr, HK_KERNEL32_MODULE, "CloseHandle", VirtualFileSystem::CloseHandle, &VirtualFileSystem::ogCloseHandle);
	IATHook32<adReadFile>(msvcr, HK_KERNEL32_MODULE, "ReadFile", VirtualFileSystem::ReadFile, &VirtualFileSystem::ogReadFile);
}

HANDLE WINAPI VirtualFileSystem::CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE hFile;
	if (dataEnVFS && _wcsicmp(lpFileName, L"data\\data_en.dat") == 0) {
		hFile = ogCreateFileW(L"data\\data_en.lst", dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		RamFS::Tracker* view = dataEnVFS->CreateTracker();
		openViews->insert(RamVFS_ViewMap::value_type(hFile, view));
	}
	else {
		hFile = ogCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	//std::wcout << "CreateFileW: " << lpFileName << "\n";
	return hFile;
}


HANDLE WINAPI VirtualFileSystem::CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE hFile;
	if (dataEnVFS && _strcmpi(lpFileName, "data\\data_en.dat") == 0) {
		hFile = ogCreateFileA("data\\data_en.lst", dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		RamFS::Tracker* view = dataEnVFS->CreateTracker();
		openViews->insert(RamVFS_ViewMap::value_type(hFile, view));
	}
	else {
		hFile = ogCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	//std::cout << "CreateFileA: " << lpFileName << "\n";
	return hFile;
}

DWORD WINAPI VirtualFileSystem::SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	DWORD ret;
	auto it = openViews->find(hFile);
	if (it != openViews->end()) {
		RamFS::Tracker* tracker = it->second;
		uint64_t newPos = tracker->Seek(lDistanceToMove, dwMoveMethod);
		if (lpDistanceToMoveHigh)
			*lpDistanceToMoveHigh = newPos >> 32;
		ret = newPos;
	}
	else {
		ret = ogSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}

	//std::cout << "SetFilePointer: " << lDistanceToMove << "," << dwMoveMethod << "\n";
	return ret;
}

BOOL WINAPI VirtualFileSystem::SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
	DWORD ret;
	auto it = openViews->find(hFile);
	if (it != openViews->end()) {
		RamFS::Tracker* tracker = it->second;
		uint64_t seek = tracker->Seek(liDistanceToMove.QuadPart, dwMoveMethod);
		if (lpNewFilePointer)
			lpNewFilePointer->QuadPart = seek;
		ret = true;
	}
	else {
		ret = ogSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
	}
	//std::cout << "SetFilePointerEx: " << liDistanceToMove.QuadPart << "," << dwMoveMethod << "\n";
	return ret;
}

BOOL WINAPI VirtualFileSystem::CloseHandle(HANDLE hObject)
{
	auto it = openViews->find(hObject);
	if (it != openViews->end()) {
		openViews->erase(it);
		delete it->second;
	}

	DWORD ret = ogCloseHandle(hObject);
	//std::cout << "CloseHandle: " << "\n";
	return ret;
}

BOOL WINAPI VirtualFileSystem::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	DWORD ret;
	auto it = openViews->find(hFile);
	if (it != openViews->end()) {
		RamFS::Tracker* tracker = it->second;
		tracker->Read(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead);
		ret = true;
	}
	else {
		ret = ogReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
	//std::cout << "ReadFile: " << "\n";
	return ret;
}
