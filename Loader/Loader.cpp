// Loader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>


#define DLL_NAME	_TEXT("cgio_patches.dll")
#define EXE_NAME	_TEXT("steam_criminal.exe")
#define ENV_VAR		_TEXT("SteamAppId")
#define ENV_VALUE	_TEXT("409850")
#define KERNEL32	_TEXT("kernel32")

#ifdef UNICODE
#define LOAD_LIBRARY_FUNC "LoadLibraryW"
#else
#define LOAD_LIBRARY_FUNC "LoadLibraryA"
#endif

const char SHELL_CODE[] = { 0xEB, 0xFE }; // jmp -2 ; Busy Loop

int _tmain()
{
	SetEnvironmentVariable(ENV_VAR, ENV_VALUE);
	
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;
	CONTEXT ctx;
	PVOID pLibRemote, pCodeRemote;

	bool created = CreateProcess(EXE_NAME, nullptr, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);

	if(!created)
	{
		tcout << _T("Failed to Create Process: '") EXE_NAME _T("'");
		exit(1);
	}

	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(pi.hThread, &ctx);

	pCodeRemote = VirtualAllocEx(pi.hProcess, nullptr, sizeof(SHELL_CODE), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	pLibRemote = VirtualAllocEx(pi.hProcess, nullptr, sizeof(DLL_NAME), MEM_COMMIT, PAGE_READWRITE);

	DWORD written;
	WriteProcessMemory(pi.hProcess, pLibRemote, DLL_NAME, sizeof(DLL_NAME), &written);
	WriteProcessMemory(pi.hProcess, pCodeRemote, SHELL_CODE, sizeof(SHELL_CODE), &written);

	DWORD returnEip = ctx.Eip;

	ctx.Eip = (DWORD)pCodeRemote;
	ctx.ContextFlags = CONTEXT_CONTROL;
	SetThreadContext(pi.hThread, &ctx);

	ResumeThread(pi.hThread);

	HMODULE hKernel32 = GetModuleHandle(KERNEL32);
	HMODULE hLibModule;
	LPTHREAD_START_ROUTINE loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(hKernel32, LOAD_LIBRARY_FUNC));
	HANDLE hThread = CreateRemoteThread(pi.hProcess, nullptr, 0, loadLibrary, pLibRemote, 0, nullptr);
	WaitForSingleObject(hThread, -1);
	GetExitCodeThread(hThread, reinterpret_cast<LPDWORD>(&hLibModule));
	CloseHandle(hThread);

	SuspendThread(pi.hThread);

	ctx.Eip = returnEip;
	ctx.ContextFlags = CONTEXT_CONTROL;
	SetThreadContext(pi.hThread, &ctx);

	ResumeThread(pi.hThread);

	VirtualFreeEx(pi.hProcess, pLibRemote, sizeof(DLL_NAME), MEM_RELEASE);
	VirtualFreeEx(pi.hProcess, pCodeRemote, sizeof(SHELL_CODE), MEM_RELEASE);
}


