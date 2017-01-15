#include "stdafx.h"
#include "RamFS.h"
#include "apiDelegates.h"

class VirtualFileSystem {
public:
	static void Initialize();

	static RamFS* DataEn;
	static RamFS* DataJp;

private:
	static adCreateFileW ogCreateFileW;
	static adCreateFileA ogCreateFileA;
	static adSetFilePointer ogSetFilePointer;
	static adSetFilePointerEx ogSetFilePointerEx;
	static adReadFile ogReadFile;
	static adCloseHandle ogCloseHandle;

	static HANDLE WINAPI CreateFileW(
		LPCWSTR               lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
	);

	static HANDLE WINAPI CreateFileA(
		LPCSTR                lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
	);

	static DWORD WINAPI SetFilePointer(
		HANDLE                hFile,
		LONG                  lDistanceToMove,
		PLONG                 lpDistanceToMoveHigh,
		DWORD                 dwMoveMethod
	);

	static BOOL WINAPI SetFilePointerEx(
		HANDLE                hFile,
		LARGE_INTEGER         liDistanceToMove,
		PLARGE_INTEGER        lpNewFilePointer,
		DWORD                 dwMoveMethod
	);

	static BOOL WINAPI CloseHandle(
		HANDLE                hObject
	);

	static BOOL WINAPI ReadFile(
		HANDLE                hFile,
		LPVOID                lpBuffer,
		DWORD                 nNumberOfBytesToRead,
		LPDWORD               lpNumberOfBytesRead,
		LPOVERLAPPED          lpOverlapped
	);
};
