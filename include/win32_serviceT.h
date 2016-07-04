#pragma once

/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#include <windows.h>
#include <string>
#include <strsafe.h>

// NOTE: SERVICE_ACCEPT_PRESHUTDOWN is supported on Windows Vista/W2K8 and later
#define WIN32_SVC_ACCEPTED_CONTROLS  (SERVICE_ACCEPT_STOP               \
                                      | SERVICE_ACCEPT_PAUSE_CONTINUE   \
                                      | SERVICE_ACCEPT_SHUTDOWN         \
                                      | SERVICE_ACCEPT_PRESHUTDOWN      \
                                      | SERVICE_ACCEPT_POWEREVENT)

#define WIN32_SERVICES_KEY_NAME         L"System\\CurrentControlSet\\Services"
#define WIN32_SERVICES_APP_EVENTLOG_KEY L"\\EventLog\\Application"


//
// Win32ServiceT
//
// This class template implements a generic Windows service. It holds a
// reference to the real service and takes care of the common features, such
// as registering the service with the Windows Event Manager and handling
// events from the Service Control Manager. It expects the actual
// Service to provide the following member functions:
//
//  initService             - performs all initialization for starting the service
//  start                   - starts the service
//  stop                    - stops the service
//  svcCtrlPreShutdown      - prepare to shutdown (SERVICE_CONTROL_PRESHUTDOWN)
//  svcCtrlShutdown         - shutdown (SERVICE_CONTROL_SHUTDOWN)
//  svcCtrlUninstallService - uninstall (Service::SVC_CTRL_UNINSTALL_SERVICE)
//  svcCtrlStop             - SERVICE_CONTROL_STOP
//  svcCtrlPause            - SERVICE_CONTROL_PAUSE
//  svcCtrlContinue         - SERVICE_CONTROL_CONTINUE
//  svcCtrlPowerResume      - processes a PBT_APMRESUMEAUTOMATIC type power event
//  svcCtrlPowerSuspend     - processes a PBT_APMSUSPEND type power event
//  svcCtrlPowerDefault     - processes all other power events
//  svcCtrlDefault          - processes all other service control events
template <class Service>
class Win32ServiceT : public Service
{
public:
    typedef typename Service::Config        Config;
    typedef typename Config::Win32Service   Win32Service;
    typedef typename Config::Logger         Logger;

    const DWORD      m_acceptedControls;

    // Initialize with an object that provides the actual service
    Win32ServiceT();

    // Neither the copy constructor nor the copy assignment operator are implemented.
    Win32ServiceT(const Win32ServiceT& other) = delete;
    Win32ServiceT& operator=(const Win32ServiceT& rhs) = delete;

    // Neither the move constructor nor move assignment operator are implemented.
    Win32ServiceT(Win32ServiceT&& other) = delete;
    Win32ServiceT& operator=(Win32ServiceT&& rhs) = delete;

    void WINAPI svcMain(DWORD dwArgc, LPWSTR *lpszArgv);
    DWORD WINAPI svcCtrlHandler(DWORD  dwControl,
                                DWORD  dwEventType,
                                LPVOID lpEventData,
                                LPVOID lpContext);
    static void WINAPI svcMainWrapper(DWORD dwArgc, LPWSTR *lpszArgv);
    static DWORD WINAPI svcCtrlHandlerWrapper(DWORD     dwControl,
                                              DWORD     dwEventType,
                                              LPVOID    lpEventData,
                                              LPVOID    lpContext);
    void setServiceIdentity(const std::wstring& svcName,
                            const std::wstring& svcDisplayName,
                            const std::wstring& svcDescription);
    void  svcReportEvent(WORD eventType, DWORD msgID, DWORD err);
    void  svcReportEvent(WORD eventType, DWORD msgID, const std::wstring &msg);
    DWORD svcReportStatus(DWORD dwCurrentState,
                          DWORD dwWin32ExitCode,
                          DWORD dwWaitHint);
    DWORD WINAPI startWin32Service();
    DWORD WINAPI registerService(SC_HANDLE hSCM, DWORD startType, LPCTSTR path, SC_HANDLE *service);
    SC_HANDLE WINAPI openService(SC_HANDLE hSCM);
    void WINAPI deleteService();

private:
    static Win32Service    *m_win32SvcPtr;
    SERVICE_STATUS_HANDLE   m_svcStatusHandle;
    SERVICE_STATUS          m_svcStatus;
    HANDLE                  m_stopEvent;
    Logger                  m_logger;

    DWORD registerEventLog(const TCHAR *path) const;
};  // class Win32ServiceT


template <class T>
Win32ServiceT<T>::Win32ServiceT()
    : m_stopEvent(0)
    , m_acceptedControls(WIN32_SVC_ACCEPTED_CONTROLS | SVC_ACCEPTED_CONTROLS)
    , m_svcStatus{0}
{
    // Set the static pointer to this service so the static functions
    // Win32ServiceT<T>::svcMainWrapper and
    // Win32ServiceT<T>::svcCtrlHandlerWrapper can find this object.
    m_win32SvcPtr = this;

    // Tell our superclass (the *real* service) about us so it can report
    // status via svcReportStatus()
    setWin32Service(this);
}


template <class T>
DWORD Win32ServiceT<T>::registerEventLog(const TCHAR *path) const
{
    DWORD           result;
    HKEY            appEventLogKey;
    HKEY            svcEventLogKey;
    std::wstring    tmp(WIN32_SERVICES_KEY_NAME);

    tmp += WIN32_SERVICES_APP_EVENTLOG_KEY;

    // Open HKLM\System\CurrentControlSet\Services\EventLog\Application
    result = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            tmp.c_str(),
                            0,
                            KEY_ALL_ACCESS,
                            &appEventLogKey);
    if (ERROR_SUCCESS == result)
    {
        // Create the service's eventlog key
        result = ::RegCreateKeyEx(appEventLogKey,
                                  m_serviceName.c_str(),
                                  0,
                                  0,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &svcEventLogKey,
                                  0);
        if (ERROR_SUCCESS == result)
        {
            DWORD   supportedTypes = EVENTLOG_ERROR_TYPE
                | EVENTLOG_INFORMATION_TYPE
                | EVENTLOG_WARNING_TYPE;
            // Setup the Windows event log
            ::RegSetValueEx(svcEventLogKey,
                            TEXT("EventMessageFile"),
                            0,
                            REG_EXPAND_SZ,
                            reinterpret_cast<const BYTE *>(path),
                            static_cast<DWORD>(lstrlen(path) * sizeof(TCHAR) + sizeof(TCHAR)));

            ::RegSetValueEx(svcEventLogKey,
                            TEXT("TypesSupported"),
                            0,
                            REG_DWORD,
                            reinterpret_cast<BYTE *>(&supportedTypes),
                            sizeof supportedTypes);

            ::RegCloseKey(svcEventLogKey);
        }

        ::RegCloseKey(appEventLogKey);
    }

    return result;
}


// Function:
//  svcReportEvent
//
// Purpose:
//   Logs messages to the event log
//
// Parameters:
//   eventType -
//   msgID
//   err
//
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
template <class T>
void Win32ServiceT<T>::svcReportEvent(WORD eventType, DWORD msgID, DWORD err)
{
    HANDLE  hEventSource;
    TCHAR   buf[50];
    LPCTSTR args[] = {buf};
    WORD    count = sizeof args / sizeof LPCTSTR;
    HRESULT hResult;

    if ((eventType == EVENTLOG_INFORMATION_TYPE)
        || (eventType == EVENTLOG_WARNING_TYPE)
        || (eventType == EVENTLOG_ERROR_TYPE)
        || (eventType == EVENTLOG_SUCCESS))
    {
        hResult = ::StringCchPrintf((LPTSTR)buf,
                                    sizeof buf / sizeof(TCHAR),
                                    L"0x%08x",
                                    err);
        if (SUCCEEDED(hResult))
        {
            hEventSource = ::RegisterEventSource(NULL, m_serviceName.c_str());

            if (NULL != hEventSource)
            {
                ::ReportEvent(hEventSource,     // event log handle
                              eventType,        // event type
                              SVC_MSG_CATEGORY, // event category
                              msgID,            // event identifier
                              NULL,             // no security identifier
                              count,            // size of lpszStrings array
                              0,                // no binary data
                              args,             // array of strings
                              NULL);            // no binary data
                ::DeregisterEventSource(hEventSource);
            }
        }
        else
        {
            /// @todo: trace_warning("Failed to convert error %d to a string", err);
        }
    }
}   // Win32ServiceT<T>::svcReportEvent


// Function:
//  svcReportEvent
//
// Purpose:
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
//
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
template <class T>
void Win32ServiceT<T>::svcReportEvent(WORD eventType, DWORD msgID, const std::wstring &msg)
{
    HANDLE  hEventSource;
    TCHAR   buf[MAX_PATH];
    LPCTSTR args[] = {buf};
    WORD    count = sizeof args / sizeof LPCTSTR;
    //HRESULT hResult;

    if ((eventType == EVENTLOG_INFORMATION_TYPE)
        || (eventType == EVENTLOG_WARNING_TYPE)
        || (eventType == EVENTLOG_ERROR_TYPE)
        || (eventType == EVENTLOG_SUCCESS))
    {

        ::wcscpy(buf, msg.c_str());
        hEventSource = ::RegisterEventSource(NULL, m_serviceName.c_str());

        if (NULL != hEventSource)
        {
            ::ReportEvent(hEventSource,     // event log handle
                          eventType,        // event type
                          SVC_MSG_CATEGORY, // event category
                          msgID,            // event identifier
                          NULL,             // no security identifier
                          count,            // size of lpszStrings array
                          0,                // no binary data
                          args,             // array of strings
                          NULL);            // no binary data
            ::DeregisterEventSource(hEventSource);
        }
    }
}   // Win32ServiceT<T>::svcReportEvent


//
// Purpose:
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation,
//     in milliseconds
//
// Return value:
//   None
//
template <class T>
DWORD Win32ServiceT<T>::svcReportStatus(DWORD    dwCurrentState,
                                        DWORD    dwWin32ExitCode,
                                        DWORD    dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    DWORD st;

    // Fill in the SERVICE_STATUS structure.
    m_svcStatus.dwCurrentState = dwCurrentState;
    m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
    m_svcStatus.dwWaitHint = dwWaitHint;

    // Accept controls if we're running or paused
    if (SERVICE_RUNNING == dwCurrentState
        || SERVICE_PAUSED == dwCurrentState)
    {
        m_svcStatus.dwControlsAccepted = m_acceptedControls;
    }
    else
    {
        m_svcStatus.dwControlsAccepted = 0;
    }

    // Increment the checkpoint until we're running or stopped
    if ((dwCurrentState == SERVICE_RUNNING)
        || (dwCurrentState == SERVICE_STOPPED))
    {
        dwCheckPoint = 0;
        m_svcStatus.dwCheckPoint = 0;
    }
    else
    {
        m_svcStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the SCM.
    if (SetServiceStatus(m_svcStatusHandle, &m_svcStatus))
    {
        st = NO_ERROR;
    }
    else
    {
        st = GetLastError();
    }

    return st;
}   // Win32ServiceT<T>::svcReportStatus


//
// Purpose:
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
//  Description
//
//
// Return value:
//   None.
//
template <class T>
void WINAPI Win32ServiceT<T>::svcMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    DWORD   st = ERROR_SUCCESS;

    /// @todo: log_trace(LOG_TRACE_TRACE, "Starting the service");
    m_svcStatus.dwCurrentState = SERVICE_STOPPED;
    m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_svcStatus.dwServiceSpecificExitCode = 0;
    svcReportEvent(EVENTLOG_INFORMATION_TYPE,
                   SVC_MSG_REGISTERING_SCM,
                   st);

    // Register the handler function for the service
    m_svcStatusHandle = ::RegisterServiceCtrlHandlerEx(lpszArgv[0],
                                                       svcCtrlHandlerWrapper,
                                                       this);

    if(m_svcStatusHandle)
    {
        // Report initial status to the SCM
        st = svcReportStatus(SERVICE_START_PENDING, NO_ERROR, SVC_WAIT_HINT);
        /// @todo: log_trace(LOG_TRACE_TRACE, "Registered service control handler");

        if (ERROR_SUCCESS == st)
        {
            // Record start-pending event
            svcReportEvent(EVENTLOG_INFORMATION_TYPE,
                           SVC_MSG_REPORTED_START_PENDING,
                           0);
            /// @todo: log_trace(LOG_TRACE_TRACE, "Reported status service start pending");
            // Create an event. The control handler function, svcCtrlHandlerWrapper,
            // signals this event when it receives the stop control code.
            m_stopEvent = ::CreateEventW(NULL,  // default security attributes
                                         TRUE,  // manual reset event
                                         FALSE, // not signaled
                                         NULL); // no name

            if (NULL != m_stopEvent)
            {
                /// @todo: log_trace(LOG_TRACE_TRACE, "Created stop event");
                svcReportEvent(EVENTLOG_INFORMATION_TYPE,
                               SVC_MSG_INITIALIZING,
                               st);

                // Initialize the real service
                st = initService();
                if (ERROR_SUCCESS == st)
                {
                    /// @todo: log_trace(LOG_TRACE_TERSE, "Service successfully initialized");
                    svcReportEvent(EVENTLOG_INFORMATION_TYPE,
                                   SVC_MSG_INITIALIZED,
                                   st);

                    // Start the real service
                    st = start();
                    if (SUCCEEDED(st))
                    {
                        st = svcReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
                        /// @todo: log_trace(LOG_TRACE_NOTICE, "%ws is running", m_serviceName.c_str());
                        if (ERROR_SUCCESS == st)
                        {
                            // report to the event log and trace log that the service is running
                            svcReportEvent(EVENTLOG_SUCCESS,
                                           SVC_MSG_REPORTED_STARTED,
                                           0);

                            /*
                             * wait here for a stop event.
                             */
                            ::WaitForSingleObject(m_stopEvent, INFINITE);
                            /// @todo: log_trace(LOG_TRACE_NOTICE, "%ws stopped", m_serviceName.c_str());
                            svcReportEvent(EVENTLOG_SUCCESS,
                                           SVC_MSG_STOPPED,
                                           0);
                            svcReportStatus(SERVICE_STOPPED, st, 0);
                        }
                        else
                        {
                            stop();
                            /// @todo: log_trace_error("Failed to report service running: %d", st);

                            svcReportEvent(EVENTLOG_ERROR_TYPE,
                                           SVC_MSG_START_FAILURE,
                                           st);
                            svcReportStatus(SERVICE_STOPPED, st, 0);
                        }
                    }
                    else
                    {
                        /// @todo: log_trace_error("Failed to start service: 0x%08x", st);
                        svcReportEvent(EVENTLOG_ERROR_TYPE,
                                       SVC_MSG_START_FAILURE,
                                       st);
                        svcReportStatus(SERVICE_STOPPED, st, 0);
                    }
                }
                else
                {
                    svcReportEvent(EVENTLOG_ERROR_TYPE,
                                   SVC_MSG_INITIALIZATION_FAILURE,
                                   st);
                    svcReportStatus(SERVICE_STOPPED, st, 0);
                }
            }
            else
            {
                st = ::GetLastError();
                /// @todo: log_trace_error("Reporting status service start pending failed: %d", st);
                svcReportStatus(SERVICE_STOPPED, st, 0);
            }
        }
        else
        {
            /// @todo: log_trace_error("Failed to report service service start-pending: %d", st);
        }
    }
    else
    {
        st = GetLastError();
        /// @todo: log_trace_error("Failed register service: %d", st);
    }
}   // Win32ServiceT<T>::svcMain


/*
** Win32ServiceT<T>::svcCtrlHandler handles the following service controls:
**
**  SERVICE_CONTROL_PRESHUTDOWN
**  SERVICE_CONTROL_SHUTDOWN
**  SVC_CTRL_UNINSTALL_SERVICE
**  SERVICE_CONTROL_STOP
**  SERVICE_CONTROL_PAUSE
**  SERVICE_CONTROL_CONTINUE
*/
template <class T>
DWORD WINAPI Win32ServiceT<T>::svcCtrlHandler(DWORD   dwControl,
                                              DWORD   dwEventType,
                                              LPVOID  lpEventData,
                                              LPVOID  lpContext)
{
    DWORD st = ERROR_SUCCESS;

    // Handle the requested control code. Each case reports status to the
    // Windows SCM and calls the *real* service's method to handle processing
    // of that case.
    switch(dwControl)
    {
        case SERVICE_CONTROL_PRESHUTDOWN:
        case SERVICE_CONTROL_SHUTDOWN:
            // The OS is shutting down.
        {
            if (SERVICE_CONTROL_PRESHUTDOWN == dwControl)
            {
                /// @todo: log_trace(LOG_TRACE_INFO, "Preshutdown event received");
                st = svcCtrlPreShutdown(dwEventType, lpEventData, lpContext);
            }
            else
            {
                /// @todo: log_trace(LOG_TRACE_INFO, "Shutdown event received");
                st = svcCtrlShutdown(dwEventType, lpEventData, lpContext);
            }

            // FIXME: what to do if st != ERROR_SUCCESS?
            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);
            svcReportEvent(EVENTLOG_INFORMATION_TYPE, SVC_MSG_SHUTTING_DOWN, 0);
            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);
            // Signal the service to stop.
            ::SetEvent(m_stopEvent);
        }
        break;

        case SVC_CTRL_UNINSTALL_SERVICE:
        {
            /// @todo: log_trace(LOG_TRACE_INFO, "Uninstall service event received");

            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);
            svcReportEvent(EVENTLOG_SUCCESS, SVC_MSG_UNINSTALL_SERVICE, 0);
            st = svcCtrlUninstallService(dwEventType, lpEventData, lpContext);

            // tell the SCM we're stopping soon
            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);
            // Signal the service to stop.
            ::SetEvent(m_stopEvent);
        }
        break;

        case SERVICE_CONTROL_STOP:
            // The service is being stopped (no OS shutdown)
        {
            /// @todo: log_trace(LOG_TRACE_INFO, "Stop event received");

            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);
            svcReportEvent(EVENTLOG_INFORMATION_TYPE, SVC_MSG_STOPPING, 0);

            st = svcCtrlStop(dwEventType, lpEventData, lpContext);
            svcReportStatus(SERVICE_STOP_PENDING, st, SVC_WAIT_HINT);

            // Signal the service to stop.
            ::SetEvent(m_stopEvent);
        }
        break;  // case SERVICE_CONTROL_STOP

        case SERVICE_CONTROL_PAUSE:
        {
            /// @todo: log_trace(LOG_TRACE_INFO, "Pause service event received");

            svcReportStatus(SERVICE_PAUSE_PENDING, st, SVC_WAIT_HINT);
            svcReportEvent(EVENTLOG_SUCCESS, SVC_MSG_PAUSED, 0);
            st = svcCtrlPause(dwEventType, lpEventData, lpContext);

            svcReportStatus(SERVICE_PAUSED, st, 0);
        }
        break;

        case SERVICE_CONTROL_CONTINUE:
        {
            /// @todo: log_trace(LOG_TRACE_INFO, "Continue service event received");

            svcReportStatus(SERVICE_CONTINUE_PENDING, st, SVC_WAIT_HINT);
            // Ensure the reconnect event is reset
            svcReportEvent(EVENTLOG_SUCCESS, SVC_MSG_CONTINUING, 0);
            st = svcCtrlContinue(dwEventType, lpEventData, lpContext);

            // FIXME: what to do if st != ERROR_SUCCESS?
            svcReportStatus(SERVICE_RUNNING, st, 0);
        }
        break;

        case SERVICE_CONTROL_POWEREVENT:
        {
            /* handle case where the server is put into a low-power mode,
            ** or brought back from one.
            **
            ** Note that for this value of dwControl, the dwEventType can be:
            **
            **  PBT_APMPOWERSTATUSCHANGE (0xA) - ignore, related to batteries
            **  PBT_APMRESUMEAUTOMATIC (0x12) - resuming low-power state (sent every time)
            **  PBT_APMRESUMESUSPEND (0x7) - resuming from a low-power state (sent if triggered by a user)
            **  PBT_APMSUSPEND (0x4) - entering a suspended state.
            **  PBT_POWERSETTINGCHANGE (0x8013) - power setting change; ignore.
            */
            svcReportEvent(EVENTLOG_INFORMATION_TYPE, SVC_MSG_POWER_EVENT, 0);
            switch (dwEventType)
            {
                case PBT_APMRESUMEAUTOMATIC:
                {
                    /// @todo: log_trace(LOG_TRACE_INFO, "Resume power event received");

                    svcReportStatus(SERVICE_CONTINUE_PENDING, st, SVC_WAIT_HINT);
                    st = svcCtrlPowerResume(lpEventData, lpContext);

                    svcReportEvent(EVENTLOG_SUCCESS, SVC_MSG_POWER_EVENT_AUTO_RESUME, 0);
                    svcReportStatus(SERVICE_RUNNING, st, 0);
                }
                break;

                case PBT_APMSUSPEND:
                {
                    /// @todo: log_trace(LOG_TRACE_INFO, "Suspend power event received");

                    svcReportStatus(SERVICE_PAUSE_PENDING, st, SVC_POWER_SUSPEND_HINT);
                    svcReportEvent(EVENTLOG_SUCCESS, SVC_MSG_POWER_EVENT_SUSPEND, 0);
                    st = svcCtrlPowerSuspend(lpEventData, lpContext);

                    svcReportStatus(SERVICE_PAUSED, st, 0);
                }
                break;

                default:
                    /// @todo: log_trace(LOG_TRACE_INFO, "Unimplemented power event (%d) received",
                    // dwEventType);

                    st = svcCtrlPowerDefault(dwEventType, lpEventData, lpContext);
                    svcReportStatus(m_svcStatus.dwCurrentState, st, 0);
                    break;
            }
        }
        break;

        case SERVICE_CONTROL_INTERROGATE:
            /// @todo: log_trace(LOG_TRACE_INFO, "Interrogate event received");

            svcReportStatus(m_svcStatus.dwCurrentState, st, 0);
            break;

        default:
            /// @todo: log_trace(LOG_TRACE_INFO, "Received control event %d. Passing to default handler", dwControl);
            st = svcCtrlDefault(dwControl,
                                dwEventType,
                                lpEventData,
                                lpContext);
            break;
    }

    return st;
}   // Win32ServiceT<T>::svcCtrlHandler


template <class T>
void WINAPI Win32ServiceT<T>::svcMainWrapper(DWORD dwArgc, LPWSTR *lpszArgv)
{
    m_win32SvcPtr->svcMain(dwArgc, lpszArgv);
}


//
// svcCtrlHandlerWrapper
//
// This is a static member function, so we can pass its address to
// RegisterServiceCtrlHandlerEx. This way data is forwarded to the service-
// specific svcCtrlHandler function.
template <class T>
DWORD WINAPI Win32ServiceT<T>::svcCtrlHandlerWrapper(DWORD    dwControl,
                                                     DWORD    dwEventType,
                                                     LPVOID   lpEventData,
                                                     LPVOID   lpContext)
{
    DWORD result;
    result = m_win32SvcPtr->svcCtrlHandler(dwControl,
                                           dwEventType,
                                           lpEventData,
                                           lpContext);
    return result;
}


template <class T>
DWORD WINAPI Win32ServiceT<T>::startWin32Service()
{
    DWORD result;

    SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            {(LPWSTR)m_serviceName.c_str(), svcMainWrapper},
            {NULL, NULL}
        };

    // StartServiceCtrlDispatcher returns when the service has stopped.
    // The process should simply terminate when the call returns.
    /// @todo: log_trace(LOG_TRACE_NOTICE, "Starting %ws", m_serviceName.c_str());
    if (FALSE == ::StartServiceCtrlDispatcher(DispatchTable))
    {
        result = ::GetLastError();
        /// @todo: log_trace_error("Start service control dispatcher failed: %d", result);
    }
    else
    {
        result = NO_ERROR;
    }

    /// @todo: log_trace(LOG_TRACE_NOTICE, "%ws exiting: %d", m_serviceName.c_str(), result);

    return result;
}


template <class T>
DWORD WINAPI Win32ServiceT<T>::registerService(SC_HANDLE hSCM,
                                               DWORD startType,
                                               LPCTSTR path,
                                               SC_HANDLE *service)
{
    DWORD result;


    /// @todo doug: someday pass in the desired values for dwServiceType rather
    // than hard-coding them here.
    SC_HANDLE svc = ::CreateService(hSCM,                       // hSCManager
                                    m_serviceName.c_str(),      // lpServiceName
                                    m_displayName.c_str(),      // dwDisplayName
                                    SERVICE_ALL_ACCESS,         // dwDesiredAccess
                                    SERVICE_WIN32_OWN_PROCESS,  // dwServiceType
                                    startType,                  // dwStartType
                                    SERVICE_ERROR_NORMAL,       // dwErrorControl
                                    path,                       // lpBinaryPathName
                                    0,                          // lpLoadOrderGroup
                                    0,                          // lpdwTagId
                                    0,                          // lpDependencies
                                    0,                          // lpServiceStartName
                                    0);                         // lpPassword
    if (svc)
    {
        SERVICE_DESCRIPTION     svcDesc;
        // restart w/ 60 second delay
        SC_ACTION                       svcActions[3];
        // reset failure count after 1 day of clean running
        SERVICE_FAILURE_ACTIONS         svcFailureActions = {
            86400,  // reset period
            NULL,   // reboot message
            NULL,   // CreateProcess command
            sizeof svcActions / sizeof(SC_ACTION),  // No. items in svcActions
            svcActions  // SC_ACTIONS array
        };
        SERVICE_FAILURE_ACTIONS_FLAG    svcFailureActionsFlag = {TRUE};

        // Restart the service after 2 minutes for the first two failures.
        // After the third failure, take no action.
        svcActions[0].Type = SC_ACTION_RESTART;
        svcActions[0].Delay = 120000;
        svcActions[1].Type = SC_ACTION_RESTART;
        svcActions[1].Delay = 120000;
        svcActions[2].Type = SC_ACTION_NONE;
        svcActions[2].Delay = 120000;

        result = registerEventLog(path);
        svcDesc.lpDescription = (LPWSTR)m_description.c_str();
        // Add a description to the service
        ::ChangeServiceConfig2(svc, SERVICE_CONFIG_DESCRIPTION, &svcDesc);

        // Ensure the SCM can restart the service if it fails
        ::ChangeServiceConfig2(svc, SERVICE_CONFIG_FAILURE_ACTIONS, &svcFailureActions);
        ::ChangeServiceConfig2(svc, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &svcFailureActionsFlag);
        // save the service handle for later use
        *service = svc;
    }
    else
    {
        result = ::GetLastError();
        /// @todo: log_trace_error("Failed to create service. Error code: %d", result);
    }

    return result;
}


template <class T>
SC_HANDLE WINAPI Win32ServiceT<T>::openService(SC_HANDLE hSCM)
{
    return ::OpenService(hSCM, m_serviceName.c_str(), SERVICE_ALL_ACCESS);
}


template <class T>
void WINAPI Win32ServiceT<T>::deleteService()
{
	// Delete this service's event log registry key
	std::wstring    tmp(WIN32_SERVICES_KEY_NAME);

    tmp += WIN32_SERVICES_APP_EVENTLOG_KEY;
    tmp += m_serviceName.c_str();
    ::RegDeleteKey(HKEY_LOCAL_MACHINE, tmp.c_str());
}
