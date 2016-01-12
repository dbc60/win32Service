/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#include "win32_utility.h"
#include "wpce_platform.h"
#include <Windows.h>

std::wstring
Win32ErrorToWString(u32 errorCode)
{
    LPWSTR messageBuffer = 0;
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 errorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPWSTR)&messageBuffer, 0, NULL);

    if (L'\r' == messageBuffer[size - 2] && L'\n' == messageBuffer[size - 1])
    {
        // Remove trailing carriage-return newline pairs.
        messageBuffer[size - 2] = L'\0';
    }

    std::wstring message(messageBuffer, size);

    // Free the buffer. LocalFree is not available in the Windows 10 modern SDK, so use
    // "HeapFree(GetProcessHeap(), 0, messageBuffer)" instead. It's the same as LocalFree(messageBuffer);
    ::HeapFree(::GetProcessHeap(), 0, messageBuffer);

    return message;
}


void
FileHandle::closePlatformHandle(void)
{
    HANDLE h = (HANDLE)handle;
    CloseHandle(h);
    handle = nullptr;
}


void
FileHandle::openPlatformHandle(void)
{
    if (fileName.length() > 0)
    {
        if (nullptr == handle)
        {
            HANDLE h = CreateFile(fileName.c_str(),         // file name
                                  GENERIC_READ,             // desired access
                                  FILE_SHARE_READ,          // share mode
                                  0,                        // security attributes
                                  CREATE_ALWAYS,            // creation disposition
                                  FILE_ATTRIBUTE_NORMAL,    // attribute flags
                                  0);                       // handle template file
            if (INVALID_HANDLE_VALUE == h)
            {
                // TODO(doug): add an error value member and map GetLastError's value to it
                DWORD lastError = GetLastError();
                OutputDebugString(L"Failed to create file");
            }
            else
            {
                handle = (void*)h;
            }
        }
        else
        {
            // NOTE(doug): File handle is already set
            OutputDebugString(L"Unexpected handle");
        }
    }
    else
    {
        // TODO(doug): flag an error. There's no file name to open
    }
}
