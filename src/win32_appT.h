#if !defined(WIN32_APPT_H)
/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#include "messages.h"
#include <windows.h>
#include <aclapi.h>
#include <winerror.h>
#include <string>
#include <system_error>


// Include this header in the source file for the service to be implemented. It
// must be included in one source file per service, because the Win32ServiceT
// class template contains two static member functions and those functions must
// be defined only once per service.


// These are service control codes accepted by all services. Others are defined by
// the specific service being implemented.
// NOTE: SERVICE_ACCEPT_PRESHUTDOWN is supported on Windows Vista/W2K8 and later
#define BASIC_SVC_ACCEPTED_CONTROLS  (SERVICE_ACCEPT_STOP               \
                                      | SERVICE_ACCEPT_PAUSE_CONTINUE   \
                                      | SERVICE_ACCEPT_SHUTDOWN         \
                                      | SERVICE_ACCEPT_PRESHUTDOWN      \
                                      | SERVICE_ACCEPT_POWEREVENT)

#define WIN32_SERVICES_KEY_NAME         L"System\\CurrentControlSet\\Services"
#define WIN32_SERVICES_APP_EVENTLOG_KEY L"\\EventLog\\Application"


//
// class template BackupServiceT.
//
//  This is a Windows Service. As a class template it must provide an interface for handling
// events from the Windows Service Manager. It also expects certain types to be defined
// in the configuration class provided in its template argument. These types are:
//
//      Config          - a convenience type for accessing the other types
//      Win32Service    - the class that manages the boilerplate interface to the Windows Service
//                        Manager.
//
//  This template must also provide an enumeration, most of which are defined by a message catalog,
// so events can be recorded in the Windows Event log. At a minimum the enumeration must include:
//
//      SVC_WAIT_HINT                   - The number of milliseconds to use as a wait hint while the service is starting.
//      SVC_CTRL_UNINSTALL_SERVICE      - The value (128, usually?) of the control to uninstall the service. The service
//                                        should stop running, so it may be uninstalled.
//      SVC_ACCEPTED_CONTROLS           - Usually set to zero. The Win32ServiceT class template defined the boilerplate
//                                        controls accepted by the service (such as SERVICE_ACCEPT_STOP,
//                                        SERVICE_ACCEPT_PAUSE_CONTINUE, SERVICE_ACCEPT_SHUTDOWN, etc.)
//      SVC_POWER_SUSPEND_HINT          - The number of milliseconds to use as a wait hint when the host is entering the
//                                        suspend state.
//
//  The remaining values in the enumeration are the values of Event Log messages. They are defined in a message catalog
// and assigned to the following constants:
//
//      SVC_MSG_UNINSTALL_SERVICE       - the message for uninstalling the servce
//      SVC_MSG_CATEGORY                - the category for the messages; needed to report an event to the event log.
//      SVC_MSG_INITIALIZING            - initializing the service
//      SVC_MSG_INITIALIZED
//      SVC_MSG_REGISTERING_SCM
//      SVC_MSG_REGISTERED_SCM
//      SVC_MSG_REPORTED_START_PENDING
//      SVC_MSG_REPORTED_STARTED
//      SVC_MSG_SCM_REPORT_FAILURE
//      SVC_MSG_REGISTRATION_FAILURE
//      SVC_MSG_INITIALIZATION_FAILURE
//      SVC_MSG_START_FAILURE
//      SVC_MSG_SHUTTING_DOWN
//      SVC_MSG_STOPPING
//      SVC_MSG_STOPPED
//      SVC_MSG_PAUSED
//      SVC_MSG_CONTINUING
//      SVC_MSG_POWER_EVENT
//      SVC_MSG_POWER_EVENT_AUTO_RESUME
//      SVC_MSG_POWER_EVENT_SUSPEND


#define SVC_NAME                        L"win32_pipe_example"
#define SVC_DISPLAY_NAME                L"Win32 Pipe Client Example"
#define SVC_DESCRIPTION                 L"Opens a named pipe and waits for a client to connect."
#define SVC_REG_KEY_NAME                L"System\\CurrentControlSet\\Services"
#define SVC_TRACE_LEVEL_PARAM           L"TraceLevel"
#define SVC_POLLING_INTERVAL_PARAM      L"PollingInterval"
#define SVC_DEFAULT_POLLING_INTERVAL    60


#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

// TODO(doug): make this into something more than a bag-o'-bits.
typedef struct
{
   OVERLAPPED oOverlap;
   HANDLE hPipeInst;
   TCHAR chRequest[BUFSIZE];
   DWORD cbRead;
   TCHAR chReply[BUFSIZE];
   DWORD cbToWrite;
} PIPEINST, *LPPIPEINST;

struct OverlappedPipe
{
    HANDLE      pipe;
    OVERLAPPED  overlapped;
    BOOL        pendingIO;
};


//
template <class Config_>
class AppT
{
private:
    OverlappedPipe  m_op;
    DWORD           m_wait;
    HANDLE          m_connectThread;

public:
    typedef Config_ Config;

    AppT();

    enum
    {
        SVC_WAIT_HINT                   = 30000,        // 30s
        SVC_CTRL_UNINSTALL_SERVICE      = 128,
        // TODO(doug): Consider adding SERVICE_ACCEPT_SESSIONCHANGE to SVC_ACCEPTED_CONTROLS to monitor logins
        SVC_ACCEPTED_CONTROLS           = 0,            // none beyond the basics
        SVC_POWER_SUSPEND_HINT          = 2000,
        SVC_MSG_UNINSTALL_SERVICE       = WPCE_SVC_MSG_UNINSTALL_SERVICE,
        SVC_MSG_CATEGORY                = WPCE_SVC_MSG_CATEGORY,
        SVC_MSG_INITIALIZING            = WPCE_SVC_MSG_INITIALIZING,
        SVC_MSG_INIT_LOGGING_FAILED     = WPCE_SVC_MSG_INIT_LOGGING_FAILED,
        SVC_MSG_INITIALIZED             = WPCE_SVC_MSG_INITIALIZED,
        SVC_MSG_REGISTERING_SCM         = WPCE_SVC_MSG_REGISTERING_SCM,
        SVC_MSG_REGISTERED_SCM          = WPCE_SVC_MSG_REGISTERED_SCM,
        SVC_MSG_REPORTED_START_PENDING  = WPCE_SVC_MSG_REPORTED_START_PENDING,
        SVC_MSG_REPORTED_STARTED        = WPCE_SVC_MSG_REPORTED_STARTED,
        SVC_MSG_SCM_REPORT_FAILURE      = WPCE_SVC_MSG_SCM_REPORT_FAILURE,
        SVC_MSG_REGISTRATION_FAILURE    = WPCE_SVC_MSG_REGISTRATION_FAILURE,
        SVC_MSG_INITIALIZATION_FAILURE  = WPCE_SVC_MSG_INITIALIZATION_FAILURE,
        SVC_MSG_START_FAILURE           = WPCE_SVC_MSG_START_FAILURE,
        SVC_MSG_SHUTTING_DOWN           = WPCE_SVC_MSG_SHUTTING_DOWN,
        SVC_MSG_STOPPING                = WPCE_SVC_MSG_STOPPING,
        SVC_MSG_STOPPED                 = WPCE_SVC_MSG_STOPPED,
        SVC_MSG_PAUSED                  = WPCE_SVC_MSG_PAUSED,
        SVC_MSG_CONTINUING              = WPCE_SVC_MSG_CONTINUING,
        SVC_MSG_POWER_EVENT             = WPCE_SVC_MSG_POWER_EVENT,
        SVC_MSG_POWER_EVENT_AUTO_RESUME = WPCE_SVC_MSG_POWER_EVENT_AUTO_RESUME,
        SVC_MSG_POWER_EVENT_SUSPEND     = WPCE_SVC_MSG_POWER_EVENT_SUSPEND,
    };


    DWORD registerService();
    DWORD unregister();
    DWORD initService();
    DWORD start();
    DWORD stop();

    DWORD svcCtrlPreShutdown(DWORD dwEventType,
                             LPVOID  lpEventData,
                             LPVOID  lpContext);

    DWORD svcCtrlShutdown(DWORD dwEventType,
                          LPVOID  lpEventData,
                          LPVOID  lpContext);

    DWORD svcCtrlUninstallService(DWORD dwEventType,
                                  LPVOID  lpEventData,
                                  LPVOID  lpContext);

    DWORD svcCtrlStop(DWORD dwEventType,
                      LPVOID  lpEventData,
                      LPVOID  lpContext);

    DWORD svcCtrlPause(DWORD dwEventType,
                       LPVOID  lpEventData,
                       LPVOID  lpContext);

    DWORD svcCtrlContinue(DWORD dwEventType,
                          LPVOID  lpEventData,
                          LPVOID  lpContext);

    DWORD svcCtrlPowerResume(LPVOID  lpEventData,
                             LPVOID  lpContext);

    DWORD svcCtrlPowerSuspend(LPVOID  lpEventData,
                              LPVOID  lpContext);

    DWORD svcCtrlPowerDefault(DWORD dwEventType,
                              LPVOID  lpEventData,
                              LPVOID  lpContext);

    DWORD svcCtrlDefault(DWORD dwControl,
                         DWORD dwEventType,
                         LPVOID  lpEventData,
                         LPVOID  lpContext);

protected:
    // Protected to ensure it's safe to make the destructor non-virtual. It's impossible to call delete on a
    // pointer to this class. The delete keyword can only be applied to pointers of derived classes (such as
    // Win32ServiceT<AppT>).
    ~AppT() {}
    const std::wstring          m_serviceName;
    const std::wstring          m_displayName;
    const std::wstring          m_description;


    // Neither the copy constructor nor the assignment operator are implemented
    AppT(const AppT& other);
    AppT& operator=(AppT& rhs);
};

void connectToNewClient();


template <class T>
AppT<T>::AppT()
        : m_serviceName(SVC_NAME)
        , m_displayName(SVC_DISPLAY_NAME)
        , m_description(SVC_DESCRIPTION)
        , m_wait(0)
        , m_connectThread(INVALID_HANDLE_VALUE)
{
    m_op.pipe = INVALID_HANDLE_VALUE;
    m_op.overlapped = {};
    m_op.pendingIO = FALSE;
}


/*
 * This function handles application-specific registration with Windows. In this case, we need an interactive
 * service. Here we open the registry and add a link to our GUI application to
 * "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run". We will remove this entry via the
 * unregister function.
 */
template <class T>
DWORD AppT<T>::registerService()
{
    return 0;
}


template <class T>
DWORD AppT<T>::unregister()
{
    return 0;
}


// IMPORTANT(doug): Using SECURITY_WORLD_RID in this function opens access to everyone. It was just a
// temporary means to get the pipe working. I think SECURITY_AUTHENTICATED_USER_RID is okay, and will only
// allow access to authenticated users, rather than anyone connecting to the host.
void
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
    ea.Trustee.ptstrName  = (LPWSTR)everyone_sid;

    PACL acl = NULL;
    ::SetEntriesInAcl(1, &ea, NULL, &acl);

    PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)::LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
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
                               BUFSIZE*sizeof(TCHAR),    // output buffer size
                               BUFSIZE*sizeof(TCHAR),    // input buffer size
                               PIPE_TIMEOUT,             // client time-out
                               &sa);                    // default security attributes
    if (INVALID_HANDLE_VALUE == op->pipe)
    {
        DWORD lastError = ::GetLastError();
        std::error_code ec(lastError, std::generic_category());

        // TODO(doug): Log the error before throwing
        throw std::system_error(ec);
    }
}


// Create a named pipe for console and GUI tools to connect to this service
template <class T>
DWORD AppT<T>::initService()
{
    HANDLE connectEvent;

    connectEvent = CreateEvent(NULL,  // default security attribute (TODO(doug): for now)
                               TRUE,  // manual reset event
                               TRUE,  // initial state = signaled per MSDN docs
                               NULL); // unnamed event object

    // TODO(doug): Verify whether throwing on error is a good/bad C++ habit
    if (NULL == connectEvent)
    {
        DWORD lastError = ::GetLastError();
        std::error_code ec(lastError, std::generic_category());

        // TODO(doug): Log the error before throwing
        throw std::system_error(ec);
    }

    m_op.overlapped.hEvent = connectEvent;

    // Create one instance of the named pipe
    ::createClientPipe(&m_op);
    // namedPipeHandle = CreateNamedPipe(SERVICE_PIPE_NAME,                        // lpName,
    //                                   PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,   // dwOpenMode,
    //                                   dwPipeMode,
    //                                   nMaxInstances,
    //                                   nOutBufferSize,
    //                                   nInBufferSize,
    //                                   nDefaultTimeOut,
    //                                   lpSecurityAttributes);

    return 0;
}


void
connectToNewClient(OverlappedPipe *op)
{
    BOOL    isConnected;
    DWORD   lastError;

    isConnected = ConnectNamedPipe(op->pipe, &op->overlapped);

    lastError = ::GetLastError();
    if (isConnected)
    {
        // The asynchronous connection failed?
        // TODO(doug): Log the error
    }
    else
    {
        switch (lastError)
        {
            case ERROR_IO_PENDING:
            {
                // The overlapped connection is in progress
                op->pendingIO = TRUE;
            } break;

            case ERROR_PIPE_CONNECTED:
            {
                if (SetEvent(op->overlapped.hEvent))
                {
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


VOID
GetAnswerToRequest(LPPIPEINST pipe)
{
    //_tprintf( TEXT("[%p] %s\n"), pipe->hPipeInst, pipe->chRequest);
    //StringCchCopy( pipe->chReply, BUFSIZE, TEXT("Default answer from server") );
    //pipe->cbToWrite = (lstrlen(pipe->chReply)+1)*sizeof(TCHAR);
}


// DisconnectAndClose(LPPIPEINST) This routine is called when an error occurs or the client closes its handle
// to the pipe.
VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
    // Disconnect the pipe instance.

    if (! DisconnectNamedPipe(lpPipeInst->hPipeInst) )
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Close the handle to the pipe instance.

    CloseHandle(lpPipeInst->hPipeInst);

    // Release the storage for the pipe instance.

    if (lpPipeInst != NULL)
        GlobalFree(lpPipeInst);
}


VOID WINAPI CompletedWriteRoutine(DWORD dwErr,
                                  DWORD cbWritten,
                                  LPOVERLAPPED lpOverLap);


// CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED) This routine is called as an I/O completion routine after
// reading a request from the client. It gets data and writes it to the pipe.
VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead,
                                 LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fWrite = FALSE;

    // lpOverlap points to storage for this instance.

    lpPipeInst = (LPPIPEINST) lpOverLap;

    // The read operation has finished, so write a response (if no
    // error occurred).

    if ((dwErr == 0) && (cbBytesRead != 0))
    {
        GetAnswerToRequest(lpPipeInst);

        fWrite = WriteFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chReply,
            lpPipeInst->cbToWrite,
            (LPOVERLAPPED) lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedWriteRoutine);
    }

    // Disconnect if an error occurred.

    if (! fWrite)
        DisconnectAndClose(lpPipeInst);
}


// CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED) This routine is called as a completion routine after
// writing to the pipe, or when a new client has connected to a pipe instance. It starts another read
// operation.
VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten,
                                  LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fRead = FALSE;

    // lpOverlap points to storage for this instance.

    lpPipeInst = (LPPIPEINST) lpOverLap;

    // The write operation has finished, so read the next request (if
    // there is no error).

    if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
        fRead = ReadFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chRequest,
            BUFSIZE*sizeof(TCHAR),
            (LPOVERLAPPED) lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE) CompletedReadRoutine);

    // Disconnect if an error occurred.

    if (! fRead)
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

    while (1)
    {
        wait = WaitForSingleObjectEx(op->overlapped.hEvent,
                                     INFINITE,
                                     TRUE);

        switch (wait)
        {
            case 0:
            {
                // The wait conditions are satisfied by a completed connect operation. If an operation is pending,
                // get the result of the connect operation
                if (op->pendingIO)
                {
                    result = ::GetOverlappedResult(op->pipe,              // pipe handle
                                                   &op->overlapped,   // OVERLAPPED structure
                                                   &bytesTransferred,
                                                   FALSE);              // don't wait
                    if (!result)
                    {
                        // TODO(doug): Log an error
                        result = ::GetLastError();
                    }
                    else
                    {
                        pipeInstance = (LPPIPEINST)::GlobalAlloc(GPTR, sizeof(PIPEINST));
                        if (NULL == pipeInstance)
                        {
                            // TODO(doug): log an error
                            result = ::GetLastError();
                        }
                        else
                        {
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


template <class T>
DWORD AppT<T>::start()
{
    DWORD result = 0;

    connectToNewClient(&m_op);
    ::CreateThread(NULL,                        // default security attributes
                   0,                           // default stack size
                   &::processClientConnection,  // start address
                   &m_op,                       // pass in the pipe and OVERLAPPED value
                   0,                           // thread runs immediately after creation
                   0);                          // no need for the thread ID
    return result;
}


template <class T>
DWORD AppT<T>::stop()
{
    return 0;
}

template <class T>
DWORD AppT<T>::svcCtrlPreShutdown(DWORD   dwEventType,
                                  LPVOID  lpEventData,
                                  LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlShutdown(DWORD   dwEventType,
                               LPVOID  lpEventData,
                               LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlUninstallService(DWORD   dwEventType,
                                       LPVOID  lpEventData,
                                       LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlStop(DWORD   dwEventType,
                           LPVOID  lpEventData,
                           LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlPause(DWORD   dwEventType,
                            LPVOID  lpEventData,
                            LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlContinue(DWORD   dwEventType,
                               LPVOID  lpEventData,
                               LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlPowerResume(LPVOID  lpEventData,
                                  LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlPowerSuspend(LPVOID  lpEventData,
                                   LPVOID  lpContext)
{
    return 0;
}


template <class T>
DWORD AppT<T>::svcCtrlPowerDefault(DWORD   dwEventType,
                                   LPVOID  lpEventData,
                                   LPVOID  lpContext)
{
    return 0;
}


// TODO(doug): Consider handling dwControl == SERVICE_CONTROL_SESSIONCHANGE. See
// https://msdn.microsoft.com/en-us/library/aa383828(v=VS.85).aspx (WM_WTSSESSION_CHANGE message) for details
// about the values dwEventType will take (like WTS_SESSION_LOGON to indicate a user has logged on to the
// session identfied by lpEventData).
template <class T>
DWORD AppT<T>::svcCtrlDefault(DWORD   dwControl,
                              DWORD   dwEventType,
                              LPVOID  lpEventData,
                              LPVOID  lpContext)
{
    return 0;
}


#define WIN32_APPT_H
#endif
