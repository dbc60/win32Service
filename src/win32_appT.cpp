#include "win32_appT.h"

static VOID GetAnswerToRequest(LPPIPEINST pipe);
static VOID DisconnectAndClose(LPPIPEINST lpPipeInst);
static VOID WINAPI CompletedWriteRoutine(DWORD dwErr,
                                         DWORD cbWritten,
                                         LPOVERLAPPED lpOverLap);


// IMPORTANT(doug): Using SECURITY_WORLD_RID in this function opens access to everyone. It was just a
// temporary means to get the pipe working. I think SECURITY_AUTHENTICATED_USER_RID is okay, and will only
// allow access to authenticated users, rather than anyone connecting to the host.
static void
CreateSecurityAttributes(SECURITY_ATTRIBUTES& sa)
{
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID everyone_sid = NULL;

    ::AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &everyone_sid);
    //::AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_AUTHENTICATED_USER_RID, 0, 0, 0, 0, 0, 0, 0, &everyone_sid);

    EXPLICIT_ACCESS ea;
    ::ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPWSTR)everyone_sid;

    PACL acl = NULL;
    ::SetEntriesInAcl(1, &ea, NULL, &acl);

    PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    ::InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    ::SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE);

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = sd;
    sa.bInheritHandle = FALSE;
}


void
createClientPipe(OverlappedPipe *op)
{
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
    SECURITY_ATTRIBUTES sa;

    ::CreateSecurityAttributes(sa);
    op->pipe = ::CreateNamedPipe(lpszPipename,             // pipe name
                                 PIPE_ACCESS_DUPLEX |      // read/write access
                                 FILE_FLAG_OVERLAPPED,     // overlapped mode
                                 PIPE_TYPE_MESSAGE |       // message-type pipe
                                 PIPE_READMODE_MESSAGE |   // message read mode
                                 PIPE_WAIT,                // blocking mode
                                 PIPE_UNLIMITED_INSTANCES, // unlimited instances
                                 BUFSIZE * sizeof(TCHAR),    // output buffer size
                                 BUFSIZE * sizeof(TCHAR),    // input buffer size
                                 PIPE_TIMEOUT,             // client time-out
                                 &sa);                    // default security attributes
    if (INVALID_HANDLE_VALUE == op->pipe) {
        DWORD lastError = ::GetLastError();
        std::error_code ec(lastError, std::generic_category());

        // TODO(doug): Log the error before throwing
        throw std::system_error(ec);
    }
}


void
connectToNewClient(OverlappedPipe *op)
{
    BOOL    isConnected;
    DWORD   lastError;

    isConnected = ConnectNamedPipe(op->pipe, &op->overlapped);

    lastError = ::GetLastError();
    if (isConnected) {
        // The asynchronous connection failed?
        // TODO(doug): Log the error
    }
    else {
        switch (lastError) {
        case ERROR_IO_PENDING:
        {
            // The overlapped connection is in progress
            op->pendingIO = TRUE;
        } break;

        case ERROR_PIPE_CONNECTED:
        {
            if (SetEvent(op->overlapped.hEvent)) {
                break;
            }
        }

        default:
        {
            // TODO(doug): Log a connection error
        } break;
        }
    }
}


static VOID
GetAnswerToRequest(LPPIPEINST pipe)
{
    //_tprintf( TEXT("[%p] %s\n"), pipe->hPipeInst, pipe->chRequest);
    //StringCchCopy( pipe->chReply, BUFSIZE, TEXT("Default answer from server") );
    //pipe->cbToWrite = (lstrlen(pipe->chReply)+1)*sizeof(TCHAR);
}


// DisconnectAndClose(LPPIPEINST) This routine is called when an error occurs or the client closes its handle
// to the pipe.
static VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
    // Disconnect the pipe instance.

    if (!DisconnectNamedPipe(lpPipeInst->hPipeInst)) {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Close the handle to the pipe instance.

    CloseHandle(lpPipeInst->hPipeInst);

    // Release the storage for the pipe instance.

    if (lpPipeInst != NULL)
        GlobalFree(lpPipeInst);
}


// CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED) This routine is called as an I/O completion routine after
// reading a request from the client. It gets data and writes it to the pipe.
static VOID WINAPI CompletedReadRoutine(DWORD dwErr,
                                        DWORD cbBytesRead,
                                        LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fWrite = FALSE;

    // lpOverlap points to storage for this instance.

    lpPipeInst = (LPPIPEINST)lpOverLap;

    // The read operation has finished, so write a response (if no
    // error occurred).

    if ((dwErr == 0) && (cbBytesRead != 0)) {
        GetAnswerToRequest(lpPipeInst);

        fWrite = WriteFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chReply,
            lpPipeInst->cbToWrite,
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
    }

    // Disconnect if an error occurred.

    if (!fWrite)
        DisconnectAndClose(lpPipeInst);
}


// CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED) This routine is called as a completion routine after
// writing to the pipe, or when a new client has connected to a pipe instance. It starts another read
// operation.
static VOID WINAPI CompletedWriteRoutine(DWORD dwErr,
                                         DWORD cbWritten,
                                         LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fRead = FALSE;

    // lpOverlap points to storage for this instance.

    lpPipeInst = (LPPIPEINST)lpOverLap;

    // The write operation has finished, so read the next request (if
    // there is no error).

    if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
        fRead = ReadFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chRequest,
            BUFSIZE * sizeof(TCHAR),
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);

    // Disconnect if an error occurred.

    if (!fRead)
        DisconnectAndClose(lpPipeInst);
}


DWORD
processClientConnection(LPVOID data)
{
    DWORD wait;
    DWORD bytesTransferred;
    LPPIPEINST  pipeInstance;
    DWORD result = 0;
    OverlappedPipe *op = (OverlappedPipe*)data;

    while (1) {
        wait = WaitForSingleObjectEx(op->overlapped.hEvent,
                                     INFINITE,
                                     TRUE);

        switch (wait) {
        case 0:
        {
            // The wait conditions are satisfied by a completed connect operation. If an operation is pending,
            // get the result of the connect operation
            if (op->pendingIO) {
                result = ::GetOverlappedResult(op->pipe,              // pipe handle
                                               &op->overlapped,   // OVERLAPPED structure
                                               &bytesTransferred,
                                               FALSE);              // don't wait
                if (!result) {
                    // TODO(doug): Log an error
                    result = ::GetLastError();
                }
                else {
                    pipeInstance = (LPPIPEINST)::GlobalAlloc(GPTR, sizeof(PIPEINST));
                    if (NULL == pipeInstance) {
                        // TODO(doug): log an error
                        result = ::GetLastError();
                    }
                    else {
                        pipeInstance->hPipeInst = op->pipe;

                        // Start the read operation for this client. Hote that this same routine is later used
                        // as a completion routine after a write operation.
                        pipeInstance->cbToWrite = 0;
                        ::CompletedWriteRoutine(0, 0, (LPOVERLAPPED)pipeInstance);

                        // Create new pipe instance for the next client.
                        createClientPipe(op);
                        connectToNewClient(op);
                    }
                }
            }
        } break;

        case WAIT_IO_COMPLETION:
            break;

        default:
        {
            // TODO(doug): Log/Throw an error?
            result = ::GetLastError();
        } break;
        }
    }

    return result;
}
