#pragma once

/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

// messages.h is a project-specific header file that merely includes a project-
// generated header file. For example, the contents of messages.h for the
// FunProject project might be:
//
//      #include "funproject-messages.h"
//
// The funproject-messages.h would be generated from, say, funproject-messages.mc
// by the Microsoft message compiler (mc.exe).

#include <Windows.h>
#include "platform.h"


INTERNAL_FUNCTION const std::wstring
ServiceState2String(DWORD currentState)
{
    std::wstring result;

    switch (currentState) {
    case SERVICE_CONTINUE_PENDING:
        result = L"continue pending";
        break;

    case SERVICE_PAUSE_PENDING:
        result = L"pause pending";
        break;

    case SERVICE_PAUSED:
        result = L"paused";
        break;

    case SERVICE_RUNNING:
        result = L"running";
        break;

    case SERVICE_START_PENDING:
        result = L"start pending";
        break;

    case SERVICE_STOP_PENDING:
        result = L"stop pending";
        break;

    case SERVICE_STOPPED:
        result = L"stopped";
        break;

    default:
        result = L"unrecognized";
        break;
    }

    return result;
}


// A class that controls a Windows service (Win32Service object).
template <class Service>
class Win32ServiceCtrlT
{
public:

    // The Config_ template parameter must define three classes:
    //  ServiceCtrlUnitTest - used for unit testing this class
    //  Win32Service        - a representation of a Windows service. It contains all
    //                        of the boilerplate code for interacting with the Windows
    //                        Service Manager.
    //  Service             - the class that does the real work of the particular service
    //                        under control.
    typedef typename Service::Config        Config;
    typedef typename Config::Win32Service   Win32Service;


    Win32ServiceCtrlT(Win32Service& win32Svc);

    DWORD startService();
    DWORD stopService();
    DWORD registerService(DWORD startType, LPCTSTR path);
    DWORD unregisterService();
    DWORD deleteService();
    DWORD waitServiceStart(SERVICE_STATUS_PROCESS   &ss);
    DWORD waitServiceStop(SERVICE_STATUS_PROCESS &ss);
    bool IsRegistered();
    bool IsRunning();
    void DebugLogServiceStatusProcess(const SERVICE_STATUS_PROCESS& ss);

private:
    // Windows SCM and Service members
    SC_HANDLE       m_hSCM;
    SC_HANDLE       m_hService;
    Win32Service&   m_win32Svc;
    bool            is_registered_;
    bool            is_running_;

    // Neither the copy constructor nor the copy assignment operator are implemented.
    Win32ServiceCtrlT(const Win32ServiceCtrlT& other) = delete;
    Win32ServiceCtrlT& operator=(const Win32ServiceCtrlT& rhs) = delete;
    
    // Neither the move constructor nor the move assignment operator are implemented.
    Win32ServiceCtrlT(Win32ServiceCtrlT&&) = delete;
    Win32ServiceCtrlT& operator=(Win32ServiceCtrlT&&) = delete;

    DWORD openService();
    void  closeService();

    enum
    {
        SVC_CTRL_UNINSTALL_SERVICE = Service::SVC_CTRL_UNINSTALL_SERVICE
    };
};  // Win32ServiceCtrlT


template <class T>
Win32ServiceCtrlT<T>::Win32ServiceCtrlT(Win32Service& win32Svc)
    : m_hSCM(0), m_hService(0), m_win32Svc(win32Svc), is_registered_(false), is_running_(false)
{
    SERVICE_STATUS_PROCESS  svcStatus;
    DWORD                   dwBytesNeeded;

    // Determine if the service is registered, and if it's running.
    is_registered_ = (ERROR_SUCCESS == openService()) && (0 != m_hService);


    if (is_registered_) {
        // see if the service is running
        if (::QueryServiceStatusEx(m_hService,
                                   SC_STATUS_PROCESS_INFO,
                                   (LPBYTE)&svcStatus,
                                   sizeof svcStatus,
                                   &dwBytesNeeded)) {
            is_running_ = (SERVICE_RUNNING == svcStatus.dwCurrentState);
        }
    }
}


// Open a handle to the Windows service
template <class T>
DWORD Win32ServiceCtrlT<T>::openService()
{
    DWORD result;

    if (0 == m_hSCM)
    {
        // open a handle to the SCM
        m_hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    }

    if (m_hSCM != 0)
    {
        // log_trace(LOG_TRACE_ANNOYING, "Opened the Windows Service Control Manager (handle=0x%08x)", m_hSCM);
        if (0 == m_hService)
        {
            // open a handle to the service
            m_hService = m_win32Svc.openService(m_hSCM);

            if (m_hService != 0)
            {
                // log_trace(LOG_TRACE_ANNOYING, "Opened service");
                result = ERROR_SUCCESS;
            }
            else
            {
                result = ::GetLastError();
                // We attempt to open the service during an unregister, so this isn't necessarily a bad thing
                // trace_warning("Failed to open the service: 0x%08x", result);
            }
        }
        else
        {
            // already opened
            // log_trace(LOG_TRACE_ANNOYING, "The service is already opened (handle=0x%08x)", m_hService);
            result = ERROR_SUCCESS;
        }
    }
    else
    {
        // something is wrong with the SCM
        result = ::GetLastError();
        // log_trace_error("Failed to open the Windows Service Control Manager: 0x%08x", result);
    }
    return result;
}   // Win32ServiceCtrlT<T>::openService


// Release the handle to the service
template <class T>
void Win32ServiceCtrlT<T>::closeService()
{
    if (m_hService)
    {
        ::CloseServiceHandle(m_hService);
        m_hService = 0;
    }

    if (m_hSCM)
    {
        ::CloseServiceHandle(m_hSCM);
        m_hSCM = 0;
    }
}   // Win32ServiceCtrlT<T>::closeService


template <class T>
DWORD Win32ServiceCtrlT<T>::startService()
{
    SERVICE_STATUS_PROCESS  ss;
    DWORD                   dwBytesNeeded;
    DWORD                   result;

    result = openService();

    if (ERROR_SUCCESS == result)
    {
        // log_trace(LOG_TRACE_LOUD, "Query service status (handle=0x%08x)", m_hService);
        // Check the status in case the service is not stopped.
        if (!QueryServiceStatusEx(m_hService,                       // handle to service
                                  SC_STATUS_PROCESS_INFO,           // information level
                                  (LPBYTE)&ss,                      // address of structure
                                  sizeof(SERVICE_STATUS_PROCESS),   // size of structure
                                  &dwBytesNeeded))                  // size needed if buffer is too small
        {
            result = GetLastError();
            // log_trace_error("QueryServiceStatusEx failed: %d", result);
        }
        else
        {
            DebugLogServiceStatusProcess(ss);
            // Check if the service is already running.
            if(ss.dwCurrentState != SERVICE_STOPPED
               && ss.dwCurrentState != SERVICE_STOP_PENDING)
            {
                // no need to start a service that's running
                // log_trace(LOG_TRACE_LOUD, "Service is already running (state=%d)", ss.dwCurrentState);
            }
            else
            {
                if (ss.dwCurrentState != SERVICE_STOPPED)
                {
                    // log_trace(LOG_TRACE_LOUD, "Service is not stopped (state=%d). Wait for it to stop...", ss.dwCurrentState);
                    // The service could be STOPPED or STOP_PENDING. Wait to be sure it's stopped.
                    result = waitServiceStop(ss);
                }

                if (result == NO_ERROR)
                {
                    // log_trace(LOG_TRACE_LOUD, "Starting the service (handle=0x%08x)", m_hService);
                    // Attempt to start the service.
                    if (FALSE == ::StartService(m_hService, // handle to service
                                                0,          // number of arguments
                                                NULL))      // no arguments
                    {
                        result = ::GetLastError();
                        // log_trace_error("Service failed to start: 0x%08x", result);
                    }
                    else
                    {
                        // log_trace(LOG_TRACE_LOUD, "Starting service...");
                        // the service should be in the start pending state
                        // Check the status until the service is no longer start pending.
                        if (!::QueryServiceStatusEx(m_hService,                     // handle to service
                                                    SC_STATUS_PROCESS_INFO,         // info level
                                                    (LPBYTE) &ss,                   // address of structure
                                                    sizeof(SERVICE_STATUS_PROCESS), // size of structure
                                                    &dwBytesNeeded))                // if buffer too small
                        {
                            result = GetLastError();
                            // log_trace_error("Query service failed: 0x%08x", result);
                        }
                        else
                        {
                            const std::wstring startState = ServiceState2String(ss.dwCurrentState);
                            result = waitServiceStart(ss);
                        }
                    }
                }
                else
                {
                    // log_trace_error("Could not stop the service (state=%d)", ss.dwCurrentState);
                }
            }
        }
    } else {
        /// @todo doug: log "failed to open a handle to the service"
    }

    return result;
}   // Win32ServiceCtrlT<T>::startService


template <class T>
DWORD Win32ServiceCtrlT<T>::stopService()
{
    DWORD   dwBytesNeeded;
    DWORD   result = ERROR_SUCCESS;

    result = openService();
    if (ERROR_SUCCESS == result)
    {
        SERVICE_STATUS_PROCESS ss;

        // log_trace(LOG_TRACE_LOUD, "Query service status (handle=0x%08x)", m_hService);
        // Make sure the service is not already stopped
        if (!::QueryServiceStatusEx(m_hService,
                                    SC_STATUS_PROCESS_INFO,
                                    (LPBYTE)&ss,
                                    sizeof ss,
                                    &dwBytesNeeded))
        {
            result = ::GetLastError();
            // log_trace_error("QueryServiceStatusEx failed: %d", result);
        }
        else if (SERVICE_STOPPED != ss.dwCurrentState)
        {
            // log_trace(LOG_TRACE_LOUD, "Current state = %d", ss.dwCurrentState);
            // If a stop is pending, just wait for it
            if (SERVICE_STOP_PENDING == ss.dwCurrentState)
            {
                result = waitServiceStop(ss);
                // log_trace(LOG_TRACE_LOUD,
                           // "Waited for stop pending transistion to stopped: result=%d, state=%d",
                           // result,
                           // ss.dwCurrentState);
            }
            else
            {
                // Send a stop code to the main service
                if (!::ControlService(m_hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ss))
                {
                    result = ::GetLastError();
                    // log_trace_error("Failed to send stop control code: %d", result);
                }
                else
                {
                    // log_trace(LOG_TRACE_LOUD, "Waiting for service to stop");
                    result = waitServiceStop(ss);
                }
            }
        }
    }

    return result;
}   // Win32ServiceCtrlT<T>::stopService


/*******************************************************************************************
 Procedure:		registerService

 Purpose:
	This function is used to register the service. It first uninstalls the service in case
    there is a previous installation that points to the wrong executable.

 Parameters:
	svcName		- the name of the service to register

 Return:
	HRESULT indicating the status of the call.

 Side Effects and Notes:
	none
*******************************************************************************************/
template <class T>
DWORD Win32ServiceCtrlT<T>::registerService(DWORD startType, LPCTSTR path)
{
    DWORD       result;

    // Remove any previous service since it may point to the incorrect file
    result = unregisterService();
    // log_trace(LOG_TRACE_LOUD, "UnregisterService returned: 0x%08x", result);

    if (0 == m_hSCM)
    {
        m_hSCM = ::OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
    }

    // log_trace(LOG_TRACE_ANNOYING, "SCM handle: 0x%08x", m_hSCM);
    if (m_hSCM)
    {
        result = m_win32Svc.registerService(m_hSCM, startType, path, &m_hService);

        if (ERROR_SUCCESS == result)
        {
            // log_trace(LOG_TRACE_LOUD, "Service created");
        }
        else
        {
            ::CloseServiceHandle(m_hSCM);
            m_hSCM = 0;
        }
    }
    else
    {
        result = ::GetLastError();
        // log_trace_error("Failed to open the Service Control Manager: %d", result);
    }

    return result;
}   // Win32ServiceCtrlT<T>::registerService


template <class T>
DWORD Win32ServiceCtrlT<T>::unregisterService()
{
    SERVICE_STATUS_PROCESS  svcStatus;
    DWORD                   dwBytesNeeded;
    DWORD                   result;

    ::ZeroMemory(&svcStatus, sizeof svcStatus);
    result = openService();

    if (ERROR_SUCCESS == result)
    {
        // see if the service is running
        // Make sure the service is not already stopped
        if (!::QueryServiceStatusEx(m_hService,
                                    SC_STATUS_PROCESS_INFO,
                                    (LPBYTE)&svcStatus,
                                    sizeof svcStatus,
                                    &dwBytesNeeded))
        {
            result = ::GetLastError();
        }
        else if (SERVICE_RUNNING == svcStatus.dwCurrentState)
        {
            // tell the service to stop with the Uninstall reason
            ::ControlService(m_hService, SVC_CTRL_UNINSTALL_SERVICE, (LPSERVICE_STATUS)&svcStatus);
            result = waitServiceStop(svcStatus);
        }
        else if (SERVICE_STOP_PENDING == svcStatus.dwCurrentState)
        {
            waitServiceStop(svcStatus);
        }

        // regardless of whether the service was successfully stopped, we will
        // attempt to unregister it and cleanup the Window registry.
        result = deleteService();
    }
    else
    {
        // During a 'register' command, we first attempt to unregister to clean
        // things up. We will get here if the service isn't registered, yet.
        // Failing to unregister a service that's not registered is "okay".
        result = ERROR_SUCCESS;
        // log_trace(LOG_TRACE_NOTICE, "Unregister: Service is not registered.");
    }

    return result;
}   // Win32ServiceCtrlT<T>::unregisterService


template <class T>
DWORD Win32ServiceCtrlT<T>::deleteService()
{
    DWORD       result = ERROR_SUCCESS;

    // Delete the service
    if (!::DeleteService(m_hService))
    {
        result = ::GetLastError();
    }

    m_win32Svc.deleteService();

    // Close the service handles, because the service is gone.
    closeService();
    return result;
}   // Win32ServiceCtrlT<T>::deleteService


template <class T>
DWORD Win32ServiceCtrlT<T>::waitServiceStart(SERVICE_STATUS_PROCESS   &ss)
{
    DWORD   result = NO_ERROR;
    DWORD   dwWaitTime;
    DWORD   dwBytesNeeded;
    DWORD   dwOldCheckPoint;
    ULONGLONG   startTickCount;

    // Save the tick count and initial checkpoint.
    startTickCount = GetTickCount64();
    dwOldCheckPoint = ss.dwCheckPoint;

    // Per the MSDN article "Starting a Service"
    // (see https://msdn.microsoft.com/en-us/library/windows/desktop/ms686315(v=vs.85).aspx)
    // do not wait longer than the wait hint. A good interval is
    // one-tenth the wait hint, but no less than 1 second and no
    // more than 10 seconds. All times are in milliseconds.
    dwWaitTime = ss.dwWaitHint / 10;
    if (dwWaitTime < 1000)
    {
        dwWaitTime = 1000;
    }
    else if (dwWaitTime > 10000)
    {
        dwWaitTime = 10000;
    }
    // log_trace(LOG_TRACE_INFO, "Service start wait time is: %dms", dwWaitTime);

    while (ss.dwCurrentState == SERVICE_START_PENDING
           && NO_ERROR == result)
    {
        // log_trace(LOG_TRACE_LOUD, "Start pending...");
        // log_trace(LOG_TRACE_ANNOYING, "Set wait time to %dms", dwWaitTime);
        Sleep(dwWaitTime);

        // log_trace(LOG_TRACE_LOUD, "Query service status (handle=0x%08x)", m_hService);
        // Check the status again.
        if (!QueryServiceStatusEx(m_hService,                       // handle to service
                                  SC_STATUS_PROCESS_INFO,           // info level
                                  (LPBYTE)&ss,                      // address of structure
                                  sizeof(SERVICE_STATUS_PROCESS),   // size of structure
                                  &dwBytesNeeded))                  // if buffer too small
        {
            result = GetLastError();

        }
        else
        {
            DebugLogServiceStatusProcess(ss);

            if (ss.dwCurrentState == SERVICE_RUNNING) {
                /// @todo doug: log the service is running
            } else if (ss.dwCheckPoint > dwOldCheckPoint) {
                /// @todo doug: log updating checkpoint

                // Continue to wait and check.
                startTickCount = GetTickCount64();
                dwOldCheckPoint = ss.dwCheckPoint;
            } else {
                ULONGLONG tickCount = GetTickCount64() - startTickCount;

                if (tickCount > ss.dwWaitHint) {
                    result = WAIT_TIMEOUT;
                } else {
                    /// @todo doug: log waiting for service to start
                }
            }
        }
    }

    return result;
}   // Win32ServiceCtrlT<T>::waitServiceStart


template <class T>
DWORD Win32ServiceCtrlT<T>::waitServiceStop(SERVICE_STATUS_PROCESS &ss)
{
    DWORD   result = NO_ERROR;
    DWORD   dwWaitTime;
    DWORD   dwBytesNeeded;
    DWORD   dwOldCheckPoint;
    ULONGLONG   startTickCount;

    // Save the tick count and initial checkpoint.
    startTickCount = GetTickCount64();
    dwOldCheckPoint = ss.dwCheckPoint;

    // log_trace(LOG_TRACE_LOUD, "Current state = %d", ss.dwCurrentState);
    // Wait for the service to stop.
    while (ss.dwCurrentState != SERVICE_STOPPED
           && NO_ERROR == result)
    {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.
        dwWaitTime = ss.dwWaitHint / 10;

        if (dwWaitTime < 1000)
        {
            dwWaitTime = 1000;
        }
        else if (dwWaitTime > 10000)
        {
            dwWaitTime = 10000;
        }

        // log_trace(LOG_TRACE_LOUD, "Set wait time to %dms", dwWaitTime);
        Sleep(dwWaitTime);

        // log_trace(LOG_TRACE_LOUD, "Query service status (handle=0x%08x)", m_hService);
        // Check the status until the service is no longer stop pending.
        if (!QueryServiceStatusEx(m_hService,                       // handle to service
                                  SC_STATUS_PROCESS_INFO,           // information level
                                  (LPBYTE)&ss,                      // address of structure
                                  sizeof(SERVICE_STATUS_PROCESS),   // size of structure
                                  &dwBytesNeeded))                  // size needed if buffer is too small
        {
            result = GetLastError();
            // log_trace_error("Query service failed: 0x%08x", result);
        }
        else if (SERVICE_STOPPED == ss.dwCurrentState)
        {
            // log_trace(LOG_TRACE_NOTICE, "The service is stopped");
            result = NO_ERROR;
        }
        else if ( ss.dwCheckPoint > dwOldCheckPoint )
        {
            // Continue to wait and check.
            startTickCount = GetTickCount64();
            dwOldCheckPoint = ss.dwCheckPoint;
        }
        else
        {
            ULONGLONG tickCount = GetTickCount64() - startTickCount;
            if(tickCount > ss.dwWaitHint)
            {
                result = WAIT_TIMEOUT;
                // trace_warning("Service failed to stop after %d ticks (max=%d)", tickCount, ss.dwWaitHint);
            }
            else
            {
                // log_trace(LOG_TRACE_ANNOYING, "Still waiting for service to stop...");
            }
        }
    }

    return result;
}   // Win32ServiceCtrlT<T>::waitServiceStop


template <class T>
bool Win32ServiceCtrlT<T>::IsRunning()
{
    return is_running_;
}   // Win32ServiceCtrlT<T>::IsRunning()


template <class T>
bool Win32ServiceCtrlT<T>::IsRegistered()
{
    return is_registered_;
}


template <class T>
void Win32ServiceCtrlT<T>::DebugLogServiceStatusProcess(const SERVICE_STATUS_PROCESS& ss)
{
    /**
      @todo doug: log the values of SERVICE_STATUS_PROCESS at 'debug' level:
     
        "SERVICE_STATUS_PROCESS {";
        "  dwServiceType                    : " << ss.dwServiceType;
        "  dwCurrentState                   : " << ss.dwCurrentState;
        "  dwControlsAccepted               : " << ss.dwControlsAccepted;
        "  dwWin32ExitCode                  : " << ss.dwWin32ExitCode;
        "  dwServiceSpecificationExitCode   : " << ss.dwServiceSpecificExitCode;
        "  dwCheckPoint                     : " << ss.dwCheckPoint;
        "  dwWaitHint                       : " << ss.dwWaitHint;
        "  dwProcessId                      : " << ss.dwProcessId;
        "  dwServiceFlags                   : " << ss.dwServiceFlags;
        "};";
    */
}
