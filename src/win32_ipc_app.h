#if !defined(WIN32_IPC_APP_H)

/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

/**
 *  @file      win32_ipc_app.h
 *  @brief     <add brief description>
 *  @details   <add details>
 *
 *  @author    Douglas B. Cuthbertson
 *  @bug       zarro boogs found.
 *
 *  @copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
 */

#include <string>


// TODO(doug): define constants and structs as needed. One rule: Use Windows types like HANDLE, DWORD and
// HMODULE for data returned from Win32 API calls. Use internal types like s32, u16 and r64 for data derived
// and/or calculated from other data where it won't be applied to a Win32 API.

// A name for the pipe
//#define SERVICE_PIPE_NAME L"\\\\.\\pipe\\appPipe"
#define SERVICE_PIPE_NAME L"\\\\.\\pipe\\mynamedpipe"

// Time for CallNamedPipe to wait in milliseconds
#define DEFAULT_PIPE_TIMEOUT    5000


// TODO(doug): Placeholder for the various transactions I want to define between the client and server.
enum Win32PipeMessageType
{
    Win32PipeHello,
};


struct Win32AppMessage
{
    Win32PipeMessageType type;
    std::string name;
};


#define DEFINE_APP_MESSAGE(Type, Name) Win32AppMessage  Type ## Msg = {Type, Name}
#define DECLARE_APP_MESSAGE(Type) Win32AppMessage extern Type ## Msg

DECLARE_APP_MESSAGE(Win32PipeHello);


void
ipcTransaction(wch *pipeName,
               Win32PipeMessageType type,
               void *message,
               u32 messageLength,
               void *reply,
               u32 replyBufferSize,
               u32 *bytesRead);

#define WIN32_IPC_APP_H
#endif
