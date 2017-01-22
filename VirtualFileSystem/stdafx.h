// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdlib.h>


#define _DR1(base,offset) *(uint32_t*)((uint8_t*)base + offset)
#define _DR2(b, o1, o2) _DR1(_DR1(b,o1),o2)
#define _DR3(b, o1, o2, o3) _DR1(_DR2(b,o1,o2),o3)
#define _DR4(b, o1, o2, o3, o4) _DR1(_DR3(b,o1,o2,o3),o4)
#define _DR5(b, o1, o2, o3, o4, o5) _DR1(_DR4(b,o1,o2,o3,o4),o5)


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>

#define __STRINGIFY(x) #x
#define TOSTRING(x) __STRINGIFY(x)

#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
#else
#define tcout std::cout
#endif

#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif



// TODO: reference additional headers your program requires here
