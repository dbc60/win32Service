/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

/**
 *  @file      win32_ipc_app.cpp
 *  @brief     <add brief description>
 *  @details   <add details>
 *  @author    Douglas B. Cuthbertson
 *  @bug       zarro boogs found.
 *
 *  @copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
 */

#include "wpce_platform.h"
#include "win32_ipc_app.h"
#include "win32_utility.h"
#include <locale>
#include <codecvt>
#include <string>
#include <sstream>
#include <stdexcept>
#include <Windows.h>
#include <winerror.h>
#include <Sddl.h>


// Define Win32PipeHelloMsg constant
DEFINE_APP_MESSAGE(Win32PipeHello, "Hello");


// This is the implementation for the IPC client. It is based on the client code at
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365789(v=vs.85).aspx (Transactions on Named
// Pipes).
void
ipcTransaction(wch *pipeName,
               Win32PipeMessageType type,
               void *message,
               u32 messageLength,
               void *reply,
               u32 replyBufferSize,
               u32 *bytesRead)
{
    BOOL success;
    DWORD lastError;

    success = ::CallNamedPipe(pipeName,                 // lpNamedPipeName
                              message,                  // lpInBuffer
                              messageLength,            // nInBufferSize
                              reply,                    // lpOutBuffer
                              replyBufferSize,          // nOutBufferSize
                              reinterpret_cast<DWORD*>(bytesRead),  // lpBytesRead
                              //NMPWAIT_WAIT_FOREVER);    // nTimeOut
                              10000);                   // nTimeOut - wait up to 10 seconds
    lastError = ::GetLastError();
    if (!success)
    {
        std::stringstream errorMessage;

        if (ERROR_MORE_DATA == lastError)
        {
            // TODO(doug): make a string table to centralize error messages.
            errorMessage << "Pipe reply too large for buffer.";
        }
        else if (ERROR_FILE_NOT_FOUND == lastError)
        {
            // If no instances of the named pipe exist, WaitNamedPipe will return immediately (sigh). If the
            // timeout value expires, it will fail with error ERROR_SEM_TIMEOUT.
            success = ::WaitNamedPipe(pipeName, 10000);
            lastError = GetLastError();

            if (!success)
            {
                if (ERROR_FILE_NOT_FOUND == lastError)
                {
                    errorMessage << "Pipe not found after waiting.";
                }
            }
            else
            {
                MessageBox(0, L"Pipe found", L"Pipe", MB_OK);
            }
        }
        else
        {
            errorMessage << "Pipe error.";
        }

        // setup converter
        typedef std::codecvt_utf8<wchar_t> converter_type;
        std::wstring_convert<converter_type, wchar_t> converter;

        // Use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
        std::string converted_str = converter.to_bytes(pipeName);
        errorMessage << " Pipe Name: " << converted_str << ", Message Type: " << Win32PipeHelloMsg.type
                     << ", Message: " << Win32PipeHelloMsg.name << ", Error: "
                     << lastError << ", " << converter.to_bytes(Win32ErrorToWString(lastError));

        throw std::runtime_error(errorMessage.str());
    }
    else
    {
        MessageBox(0, L"Pipe transaction successful", L"Pipe", MB_OK);
    }
}


void CALLBACK
waitCallback(PVOID param, BOOLEAN waitFired)
{
    // If waitFired is TRUE, then the wait timed out, otherwise it was signalled.

    // The param field is a pointer to any data you might want to pass. If this function is passed to
    // RegisterWaitForSingleObject, then it will be called when the event fires.
}

bool
okayToCallNamedPipe()
{
    bool result = false;
    DWORD lastError;
    HANDLE namedPipeCreatedEvent;
    SECURITY_ATTRIBUTES sa = {};
    WCHAR *stringSecurityDescriptor;
    ULONG bytesOut = 0;

    sa.nLength = sizeof sa;

    // DACL: Deny guest
    stringSecurityDescriptor = L"D:"    // Discretionary ACL
        L"(D;OICI;GA;;;BG)"             // Deny all to built-in guest
        L"(D;OICI;GA;;;AN)"             // Deny all to anonymous logon
        L"(A;OICI;GRGWGX;;;AU)";        // Allow generic read, write and execute to authenticated users
    // TODO(doug): Use a GUID for the final name of the event object, and define it in a header that's shared
    // between this file and another file with Win32 service code (for the app_service, not the
    // win32_service).  This is a manual-reset event. Once the service triggers it, it will stay triggered so
    // all instances of the app_service will know that the named pipe exists.
    if (ConvertStringSecurityDescriptorToSecurityDescriptor(stringSecurityDescriptor,
                                                            SDDL_REVISION_1,
                                                            &sa.lpSecurityDescriptor,
                                                            &bytesOut))
    {
        HANDLE registerWaitHandle = INVALID_HANDLE_VALUE;

        // TODO(doug): replace with CreateEventEx and specify an access mask.
        namedPipeCreatedEvent = CreateEvent(&sa,      // security attributes
                                            TRUE,     // leave it set once it's triggered.
                                            FALSE,    // not signalled on creation
                                            L"Global\\NamedPipeCreatedEvent"); // the pipe's name

        lastError = GetLastError();
        if (ERROR_ALREADY_EXISTS == lastError)
        {
            auto success = RegisterWaitForSingleObject(&registerWaitHandle,     // handle to a new wait object.
                                                       &namedPipeCreatedEvent,  // handle to the event
                                                       waitCallback,            // our callback function
                                                       nullptr,                 // no context
                                                       INFINITE,                // wait a long time
                                                       WT_EXECUTEONLYONCE);
            if (INVALID_HANDLE_VALUE == namedPipeCreatedEvent)
            {
                throw lastError;
            }
            else
            {
                ;
            }
        }
    }

    return result;
}
