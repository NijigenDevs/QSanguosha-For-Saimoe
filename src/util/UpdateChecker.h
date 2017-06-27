#ifndef _UPDATECHECKER_H
#define _UPDATECHECKER_H

#ifdef _WINDOWS
// for Windows Only
#ifndef _UNICODE
#define _UNICODE
#endif
#include "Shlwapi.h"
#include "tchar.h"
#include "string.h"
#include <iostream>
#pragma comment (lib, "Shlwapi.lib")
#endif

class UpdateChecker
{
public:
#ifdef _WINDOWS
    static void GetModuleDirectory(LPTSTR szPath);
    static bool UpdateAvailable(bool notifyIfNewest = false);
#endif
    static void CheckUpdate(bool notifyIfNewest = false);
};


#endif