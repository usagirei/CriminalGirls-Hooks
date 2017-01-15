#pragma once

#include "stdafx.h"

template <typename TDelegate>
bool IATHook32(HMODULE hTargetModule, char* lpModuleName, char* lpSymbolName, TDelegate newFunc, TDelegate *oldFunc) {

	if (hTargetModule == nullptr)
		hTargetModule = GetModuleHandle(nullptr);

	uint8_t* baseAddr = (uint8_t*)hTargetModule;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(baseAddr);
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)(baseAddr + dosHeader->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor =
		(PIMAGE_IMPORT_DESCRIPTOR)(baseAddr + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	//uint32_t importDescriptorSize = (uint32_t)(baseAddr + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size);

	for (IMAGE_IMPORT_DESCRIPTOR* iid = importDescriptor; iid->Name != NULL; iid++) {
		char* curModuleName = (char*)(baseAddr + iid->Name);

		int checkMod = _strcmpi(lpModuleName, curModuleName);

		if (checkMod != 0)
			continue;

		PIMAGE_THUNK_DATA32 originalThunk = (PIMAGE_THUNK_DATA32)(baseAddr + iid->OriginalFirstThunk);
		PIMAGE_THUNK_DATA32 thunk = (PIMAGE_THUNK_DATA32)(baseAddr + iid->FirstThunk);

		for (/*empty*/; thunk->u1.AddressOfData != NULL; thunk++, originalThunk++) {
			DWORD isOrdinal = thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32;
			if (isOrdinal == 0) {
				PIMAGE_IMPORT_BY_NAME funcNameStruct = (PIMAGE_IMPORT_BY_NAME)(baseAddr + originalThunk->u1.AddressOfData);

				char* curSymbolName = (char*)(funcNameStruct->Name);
				int checkFunc = _strcmpi(lpSymbolName, curSymbolName);

				if (checkFunc != 0)
					continue;

				*oldFunc = (TDelegate)thunk->u1.Function;

				PDWORD pOldFunc = &thunk->u1.Function;
				DWORD accessProtectionValue, accessProtec;
				int vProtect = VirtualProtect(pOldFunc, sizeof(PDWORD), PAGE_EXECUTE_READWRITE, &accessProtectionValue);
				*pOldFunc = (DWORD)newFunc;
				vProtect = VirtualProtect(pOldFunc, sizeof(LPDWORD), accessProtectionValue, &accessProtec);
				return true;
			}
		}
	}
	return false;
}
