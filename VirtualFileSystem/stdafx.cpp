// stdafx.cpp : source file that includes just the standard includes
// VirtualFileSystem.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <string>
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

std::wstring StringToWideString(const std::string& str, int codePage)
{
	int cstrl = str.length() + 1;
	auto temp = new wchar_t[cstrl];
	MultiByteToWideChar(codePage, 0, str.c_str(), -1, temp, cstrl);
	auto wstr = std::wstring(temp);
	delete[] temp;
	return wstr;
}

std::string GetLastErrorAsString()
{
	DWORD errorNo = GetLastError();
	if (errorNo == 0)
		return std::string();

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer,
		0,
		NULL);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}