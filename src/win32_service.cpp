/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#include "win32_serviceConfig.h"
#include "service_platform.h"
#include <windows.h>
#include <winerror.h>
#include <string>
#include <iostream>

Win32Service *Win32Service::m_win32SvcPtr;

// TODO(doug): Need to add a logging/tracing facility

// Error is something that you do, like try to read a file that doesn't exist.
// Fatal is something that's done to you, like running out of memory.
// Here's an enumeration ordered in decreasing levels of severity. That way, the higher the tracing level, the
// more messages are logged. For example, if the tracing level is set to TRACE_WARNING, then only messages
// marked as TRACE_WARNING or less are logged.
enum Tracing
{
    TRACE_FATAL,            // 0. Most severe - shutting down. Double-plus ungood!
    TRACE_ERROR,            // 1. Non-fatal errors, but the operation failed. Perhaps alert sysadmin?
    TRACE_WARNING,          // 2. Unexpected, but recoverable
    TRACE_INFO,             // 3. Good stuff to log, like start/stop, config assumptions, etc.
    TRACE_VERBOSE,          // 4. Diagnostic information for IT, system administrators
    TRACE_DEBUG,            // 5. Diagnostics for developers.
    TRACE_DEBUG_LOUD,       // 6. Detailed diagnostics for developers.
    TRACE_DEBUG_ANNOYING,   // 7. Ridiculously detailed diagnostics. Do we really need this?
    TRACE_LEVEL_COUNT       // 8. Sentinel to indicate there are no more trace levels
};

// Specify the default log/trace level
#define DEFAULT_TRACE_LEVEL TRACE_INFO

u32 usage(wch *svcName, u32 err)
{
	std::wcout << L"Usage: " << svcName << L" COMMAND ..." << std::endl;
	std::wcout << L"\"" << svcName << L" help\" for a list of available commands." << std::endl;
	std::wcout << L"\"" << svcName << L" help COMMAND\" for specific details." << std::endl;

    std::wcout << std::endl;
    return err;
}   // usage


u32 help(wch *svcName, wch *helpArg, u32 err)
{
    std::wcout << L"Usage: " << svcName;
    if (0 == helpArg)
    {
        std::wcout << L" help COMMAND" << std::endl << std:: endl << L"Available COMMANDs:" << std::endl;
        std::wcout << L"help           register       start          stop           unregister" << std::endl;
    }
    else if (::lstrcmpi(helpArg, L"help")==0)
    {
    	std::wcout << L" help COMMAND" << std::endl << std::endl << "Display information on COMMAND." << std::endl;
    }
    else if (::lstrcmpi(helpArg, L"register")==0)
    {
        std::wcout << L" register [OPTIONS}" << std::endl << std::endl;
        std::wcout << std::endl << L"Register/install the " << svcName << L" service." << std::endl;
        std::wcout << L"Options:" << std::endl;
        std::wcout << L"    --trace: specify tracing levels in the log file." << std::endl;
        std::wcout << L"             The trace level must be between 0 (errors only) and 8 (annoying)" << std::endl;
        std::wcout << L"             The default is 2 (warning)" << std::endl;
        std::wcout << L"    --start: set the start type for the service. It must be auto, demand or disabled." << std::endl;
        std::wcout << L"             The default is auto." << std::endl;
        std::wcout << L"    --polling-interval: set the interval, in seconds, at which" << std::endl;
        std::wcout << L"                        layer 3 settings will be polled for changes." << std::endl;
        std::wcout << L"                        The default is 300 seconds." << std::endl;
    }
    else if (::lstrcmpi(helpArg, L"start")==0)
    {
    	std::wcout << L" start" << std::endl << std::endl << L"start the " << svcName << L" service." << std::endl;
    }
    else if (::lstrcmpi(helpArg, L"stop")==0)
    {
    	std::wcout << L" stop" << std::endl << std::endl << L"stop the " << svcName << L" service." << std::endl;
    }
    else if (::lstrcmpi(helpArg, L"unregister")==0)
    {
    	std::wcout << L" unregister" << std::endl << std::endl << L"unregister/uninstall the " << svcName << L" service." << std::endl;
    }

    std::wcout << std::endl;
    return err;
}   // help


void
setServiceParameters(DWORD traceLevel, DWORD pollingInterval)
{
    HKEY    servicesKey, svcKey, paramKey;
    LONG        st;

    // Get the polling interval from the registry.
    st = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                         SVC_REG_KEY_NAME,
                         0,
                         KEY_ALL_ACCESS | KEY_WOW64_64KEY,
                         &servicesKey);

    if (ERROR_SUCCESS == st)
    {
        st = ::RegOpenKeyExW(servicesKey,
                             SVC_NAME,
                             0,
                             KEY_ALL_ACCESS | KEY_WOW64_64KEY,
                             &svcKey);
        if (ERROR_SUCCESS == st)
        {
            st = ::RegCreateKeyEx(svcKey,
                                  L"Parameters",
                                  0,
                                  0,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS | KEY_WOW64_64KEY,
                                  0,
                                  &paramKey,
                                  0);
            if (ERROR_SUCCESS == st)
            {
                ::RegSetValueEx(paramKey,
                                SVC_TRACE_LEVEL_PARAM,
                                0,
                                REG_DWORD,
                                reinterpret_cast<BYTE*>(&traceLevel),
                                sizeof traceLevel);
                ::RegSetValueExW(paramKey,
                                 SVC_POLLING_INTERVAL_PARAM,
                                 0,
                                 REG_DWORD,
                                 reinterpret_cast<BYTE*>(&pollingInterval),
                                 sizeof pollingInterval);
                ::RegCloseKey(paramKey);
            }

            ::RegCloseKey(svcKey);
        }

        ::RegCloseKey(servicesKey);
    }
}


int
wmain(int argc, wchar_t *argv[])
{
    Win32Service svc;
    Win32ServiceCtrl svcCtrl(svc);
    int result;

    if (argc >= 2 && argc <= 8)
    {
        // Handle command line options like start/stop/register/unregister
        if (::lstrcmpi(argv[1], L"start")==0)
        {
            result = svcCtrl.startService();
        }
        else if (::lstrcmpi(argv[1], L"stop")==0)
        {
            // remove the service
            result = svcCtrl.stopService();
		}
        else if (::lstrcmpi(argv[1], L"register")==0)
        {
            DWORD traceLevel = DEFAULT_TRACE_LEVEL;
            DWORD startType = SERVICE_AUTO_START;
            DWORD pollingInterval = SVC_DEFAULT_POLLING_INTERVAL;

            // register the service
            if (argc > 3 && (argc == 4 || argc == 6 || argc == 8))
            {
                for (int i = 2; i < argc; i +=2)
                {
                    // We have arguments for the trace level, start type, or both
                    if (0 == ::lstrcmpi(argv[i], L"--trace"))
                    {
                        // 'trace <traceLevel>'
                        traceLevel = ::_wtoi(argv[i+1]);
                        if (traceLevel < TRACE_ERROR || traceLevel >= TRACE_LEVEL_COUNT)
                        {
                            traceLevel = TRACE_WARNING;
                            usage(argv[0], ERROR_SUCCESS);
                        }
                    }

                    if (0 == ::lstrcmpi(argv[i], L"--start"))
                    {
                        // 'trace <traceLevel> start <startType>'
                        if (0 == ::lstrcmpi(argv[i+1], L"auto"))
                        {
                            startType = SERVICE_AUTO_START;
                        }
                        else if (0 == ::lstrcmpi(argv[i+1], L"demand"))
                        {
                            startType = SERVICE_DEMAND_START;
                        }
                        else if (0 == ::lstrcmpi(argv[i+1], L"disabled"))
                        {
                            startType = SERVICE_DISABLED;
                        }
                        else
                        {
                            usage(argv[0], ERROR_SUCCESS);
                        }
                    }

                    if (0 == ::lstrcmpi(argv[i], L"--polling-nterval"))
                    {
                        pollingInterval = ::_wtoi(argv[i+1]);
                    }
                }
            }
            else if (argc == 3 || argc == 5 || argc > 6)
            {
                usage(argv[0], ERROR_SUCCESS);
            }

            TCHAR path[_MAX_PATH];
            if (::GetModuleFileName(0, path, sizeof path / sizeof path[0]) > 0)
            {
                result = svcCtrl.registerService(startType, path);
                    setServiceParameters(traceLevel, pollingInterval);
                }
                else
                {
                result = ::GetLastError();
            }
        }
        else if (::lstrcmpi(argv[1], L"unregister")==0)
        {
            // remove the service
            result = svcCtrl.unregisterService();
		}
        else if (::lstrcmpi(argv[1], L"help")==0)
        {
            result = help(argv[0], (argc == 3 ? argv[2] : 0), ERROR_SUCCESS);
        }
        else if (::lstrcmpi(argv[1], L"status") == 0)
        {
            // Display whether or not the service is registered, and if it is registered, display it's current
            // state (running, stopped, paused, etc.)
            result = ERROR_SUCCESS;
        }
        else
        {
            result = usage(argv[0], ERROR_SUCCESS);
        }
    }
    else
    {
        result = svc.startWin32Service();
    }

    return result;
}
