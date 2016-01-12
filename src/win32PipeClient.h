#if !defined(WIN32_APP_H)
/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */


#include <Windows.h>
#include <winerror.h>

// The longest string we're likely to define in the resource's string table.
#define MAX_LOADSTRING 100

struct Win32State
{
    HINSTANCE instance;
    WCHAR title[MAX_LOADSTRING];              // The title bar text
    WCHAR windowClassName[MAX_LOADSTRING];    // the main window class name
    WCHAR *cmdLine;
    int cmdShow;
    HWND window;
    HACCEL acceleratorTable;
};


#define WIN32_APP_H
#endif
