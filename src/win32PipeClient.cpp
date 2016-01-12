/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

/**
 *  @file      win32_app.cpp
 *  @brief     <add brief description>
 *  @details   <add details>
 *  @author    Douglas B. Cuthbertson
 *  @bug       zarro boogs found.
 *
 *  @copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
 */

#include "win32PipeCLient.h"
#include "wpce_platform.h"
#include "win32_ipc_app.h"

// Exclude the min and max macros from windows.h
#define NOMINMAX
#include <windows.h>
#include <winerror.h>
#include <cassert>
#include "resource.h"


GLOBAL_VARIABLE b32 globalRunning;


namespace WinMainParameters
{
// Returns the value that would be passed to the first wWinMain HINSTANCE parameter or null if an error
// was encountered. If this function returns null, call ::GetLastError() to get more information.
inline HINSTANCE
GetHInstance()
{
    return static_cast<HINSTANCE>(::GetModuleHandleW(nullptr));
}


// Returns the value that would be passed to the second wWinMain HINSTANCE parameter. This function
// always return null as per the WinMain documentation.
inline HINSTANCE
GetHPrevInstance()
{
    return static_cast<HINSTANCE>(nullptr);
}


// Returns the value that would be passed to the wWinMain LPWSTR parameter. If there are no command line
// parameters, this returns a valid pointer to a null terminator character (i.e. an empty string).  Note:
// The caller must not free the returned value. Attempting to free it will cause undefined behavior.
inline LPWSTR
GetLPCmdLine()
{
    // The first argument is the program name. To allow it to have spaces, it can be surrounded by
    // quotes. We must track if the first argument is quoted since a space is also used to separate each
    // parameter.
    bool isQuoted = false;
    const wchar_t space = L' ';
    const wchar_t quote = L'\"';
    const wchar_t nullTerminator = L'\0';

    LPWSTR lpCmdLine = ::GetCommandLineW();
    assert(lpCmdLine != nullptr);

    // The lpCmdLine in a WinMain is the command line as a string excluding the program name.  Program
    // names can be quoted to allow for space characters so we need to deal with that.
    while (*lpCmdLine <= space || isQuoted)
    {
        if (*lpCmdLine == quote)
        {
            isQuoted = !isQuoted;
        }

        lpCmdLine++;
    }

    // Get past any additional whitespace between the end of the program name and the beginning of the
    // first parameter (if any). If we reach a null terminator we are done (i.e. there are no arguments
    // and the pointer itself is still properly valid).
    while (*lpCmdLine <= space && *lpCmdLine != nullTerminator)
    {
        lpCmdLine++;
    }

    // This will now be a valid pointer to either a null terminator or to the first character of the first
    // command line parameter after the program name.
    return lpCmdLine;
}


// Returns the value that would be passed to the wWinMain int parameter.
inline int
GetNCmdShow()
{
    int result;

    // It's possible that the process was started with STARTUPINFOW that could have a value for show
    // window other than SW_SHOWDEFAULT. If so, we retrieve and return that value. Otherwise, we return
    // SW_SHOWDEFAULT.
    ::STARTUPINFOW startupInfo;
    ::GetStartupInfoW(&startupInfo);
    if ((startupInfo.dwFlags & STARTF_USESHOWWINDOW) != 0)
    {
        result = startupInfo.wShowWindow;
    }
    else
    {
        result = SW_SHOWDEFAULT;
    }

    return result;
}

}   // namespace WinMainParameters


// Message handler for about box.
INT_PTR CALLBACK
About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
	}
	return (INT_PTR)FALSE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                {
                    ::DialogBox(0, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                } break;

                case IDM_EXIT:
                {
                    ::DestroyWindow(hWnd);
                } break;

                default:
                {
                    result = ::DefWindowProc(hWnd, message, wParam, lParam);
                } break;
            }
        } break;

        case WM_CLOSE:
        {
            // TODO(doug): Handle this with a message to the user?
            OutputDebugStringA("WM_CLOSE\n");
            DestroyWindow(hWnd);
        } break;

        case WM_ACTIVATEAPP:
        {
#if 0
            if (TRUE == wParam)
            {
                SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 200, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;

        case WM_DESTROY:
        {
            // TODO(doug): Handle this as an error - recreate window?
            OutputDebugStringA("WM_DESTROY\n");
            PostQuitMessage(0);
        } break;

        // Keyboard input
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // Handle the keys that aren't handled in win32ProcessPendingMessages().
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            if (hdc)
            {
                RECT clientRect;
                GetClientRect(hWnd, &clientRect);
                // HGDIOBJ objects obtained from GetStockObject do not need to be deleted with DeleteObject as
                // per the documentation: https://msdn.microsoft.com/en-us/library/dd144925(v=vs.85).aspx
                HGDIOBJ whiteBrushGDIObj = GetStockObject(WHITE_BRUSH);
                if (whiteBrushGDIObj == nullptr || GetObjectType(whiteBrushGDIObj) != OBJ_BRUSH)
                {
                    ::PostQuitMessage(1);
                }
                else
                {
                    HBRUSH whiteBrush = static_cast<HBRUSH>(whiteBrushGDIObj);
                    FillRect(hdc, &clientRect, whiteBrush);
                    COLORREF blackTextColor = 0x00000000;
                    if (SetTextColor(hdc, blackTextColor) == CLR_INVALID)
                    {
                        PostQuitMessage(1);
                    }
                    else
                    {
                        const wchar_t helloWorldString[] = L"Hello world!";
                        DrawTextW(hdc, helloWorldString, ARRAYSIZE(helloWorldString), &clientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
                    }
                }

                EndPaint(hWnd, &ps);
            }
            else
            {
                OutputDebugStringA("WM_PAINT: No device context!\n");
            }
        } break;

        default:
        {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        } break;
    }

    return result;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM
MyRegisterClass(HINSTANCE hInstance, wch *windowClassName)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32APP));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WIN32APP);
    wcex.lpszClassName = windowClassName;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL
InitInstance(Win32State& win32State)
{
    BOOL result;

    //hInst = hInstance; // Store instance handle in our global variable

    win32State.window = CreateWindow(win32State.windowClassName,
                                     win32State.title,
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     0,
                                     CW_USEDEFAULT,
                                     0,
                                     NULL,
                                     NULL,
                                     win32State.instance,
                                     NULL);

    if (!win32State.window)
    {
        // nice to have getLastError for debugging
        DWORD getLastError = ::GetLastError();
        result = FALSE;
    }
    else
    {
        ShowWindow(win32State.window, win32State.cmdShow);
        UpdateWindow(win32State.window);

        result = TRUE;
    }

    return result;
}


// Bit 30 is the previous Win32State. It is always 1 for a WM_KEYUP message.
#define KEY_MSG_WAS_DOWN_BIT (1 << 30)

// Bit 31 is transition Win32State. It is always 0 for a WM_KEYDOWN message.
#define KEY_MSG_IS_DOWN_BIT (1 << 31)

// For WM_SYSKEYDOWN messages, bit 29 is 1 if the ALT key is down.
#define KEY_MSG_ALT_KEY_DOWN_BIT (1 << 29)

INTERNAL_FUNCTION void
win32ProcessPendingMessages(Win32State *state)
{
    MSG message;
    while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
    {
        if (!TranslateAccelerator(message.hwnd, state->acceleratorTable, &message))
        {
            switch (message.message)
            {
                case WM_QUIT:
                {
                    globalRunning = false;
                } break;

                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP:
                {
                    uint32 vkCode = static_cast<uint32>(message.wParam);

                    // NOTE(doug): Since we are comparing wasDown to isDown, we MUST use == and != to convert
                    // these bit tests to actual 0 or 1 values.
                    bool32 wasDown = ((message.lParam & KEY_MSG_WAS_DOWN_BIT) != 0);
                    bool32 isDown = ((message.lParam & KEY_MSG_IS_DOWN_BIT) == 0);
                    bool32 altKeyIsDown = (message.lParam & KEY_MSG_ALT_KEY_DOWN_BIT);

                    if (wasDown != isDown)
                    {
                        switch (vkCode)
                        {
                            case 'W':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->moveUp, isDown);
                            } break;

                            case 'A':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->moveLeft, isDown);
                            } break;

                            case 'S':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->moveDown, isDown);
                            } break;

                            case 'D':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->moveRight, isDown);
                            } break;

                            case 'Q':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->leftShoulder, isDown);
                            } break;

                            case 'E':
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->rightShoulder, isDown);
                            } break;

                            case VK_UP:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->actionUp, isDown);
                            } break;

                            case VK_LEFT:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->actionLeft, isDown);
                            } break;

                            case VK_DOWN:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->actionDown, isDown);
                            } break;

                            case VK_RIGHT:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->actionRight, isDown);
                            } break;

                            case VK_ESCAPE:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->start, isDown);
                            } break;

                            case VK_SPACE:
                            {
                                //win32ProcessKeyboardMessage(&keyboardController->back, isDown);
                            } break;
#if APP_INTERNAL
                            case 'P':
                            {
                                if (isDown)
                                {
                                    globalPause = !globalPause;
                                }
                            } break;

                            case 'L':
                            {
                                if (isDown)
                                {
                                    if (state->inputPlayingIndex == 0)
                                    {
                                        // The recording is not playing
                                        if (state->inputRecordingIndex == 0)
                                        {
                                            // Start recording
                                            win32BeginRecordingInput(state, 1);
                                        }
                                        else
                                        {
                                            // Stop recording and start playback
                                            win32EndRecordingInput(state);
                                            win32BeginInputPlayBack(state, 1);
                                        }
                                    }
                                    else
                                    {
                                        // Stop playing the recording to return control
                                        // to the game controllers.
                                        win32EndInputPlayBack(state);
                                    }
                                }

                            } break;
#endif
                            case VK_F4:
                            {
                                // End the application with ALT-F4
                                if (isDown && altKeyIsDown)
                                {
                                    globalRunning = false;
                                }
                            } break;

                            case VK_RETURN:
                            {
                                if (isDown && altKeyIsDown)
                                {
                                    if (message.hwnd)
                                    {
                                        //toggleFullscreen(message.hwnd);
                                    }
                                }
                            } break;

                            default:
                            {
                                // Handle all the other keys
                                TranslateMessage(&message);
                                DispatchMessageW(&message);
                            } break;
                        }   // switch (vkCode)
                    }
                } break;
                default:
                {
                    TranslateMessage(&message);
                    DispatchMessageW(&message);
                } break;
            }
        }
        else
        {
            OutputDebugStringA("Accelerator pressed.\n");
        }
    }
}


// ISO C++ conformant entry point. The project properties explicitly sets this as the entry point in the manner
// documented for the linker's /ENTRY option: http://msdn.microsoft.com/en-us/library/f9t8842e.aspx . As per
// the documentation, the value set as the entry point is "mainCRTStartup", not "main". Every C or C++ program
// must perform any initialization required by the language standards before it begins executing our code. In
// Visual C++, this is done by the *CRTStartup functions, each of which goes on to call the developer's entry
// point function.
int
main(int /*argc*/, char* /*argv*/[])
{
    int result = 0;
    Win32State win32State = {};

    // Use the functions from WinMainParameters.h to get the values that would've been passed to WinMain.
    // Note that these functions are in the WinMainParameters namespace.
    win32State.instance = WinMainParameters::GetHInstance();
    //HINSTANCE hPrevInstance = WinMainParameters::GetHPrevInstance();
    win32State.cmdLine = WinMainParameters::GetLPCmdLine();
    win32State.cmdShow = WinMainParameters::GetNCmdShow();

    // Assert that the values returned are expected.
    assert(win32State.instance != nullptr);
    //assert(hPrevInstance == nullptr);
    assert(win32State.cmdLine != nullptr);

    // Close the console window. This is not required, but if you do not need the console then it should be
    // freed in order to release the resources it is using. If you wish to keep the console open and use it
    // you can remove the call to FreeConsole. If you want to create a new console later you can call
    // AllocConsole. If you want to use an existing console you can call AttachConsole.
    //FreeConsole();

    // ***********************
    // If you want to avoid creating a console in the first place, you can change the linker /SUBSYSTEM
    // option in the project properties to WINDOWS as documented here:
    // http://msdn.microsoft.com/en-us/library/fcc1zstk.aspx . If you do that you should comment out the
    // above call to FreeConsole since there will not be any console to free. The program will still
    // function properly. If you want the console back, change the /SUBSYSTEM option back to CONSOLE.
    // ***********************

    // Note: The remainder of the code in this file comes from the default Visual C++ Win32 Application
    // template (with a few minor alterations). It serves as an example that the program works, not as an
    // example of good, modern C++ code style.

    //UNREFERENCED_PARAMETER(hPrevInstance);
    //UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    ::LoadString(win32State.instance, IDS_APP_TITLE, win32State.title, MAX_LOADSTRING);
    ::LoadString(win32State.instance, IDC_WIN32APP, win32State.windowClassName, MAX_LOADSTRING);
    MyRegisterClass(win32State.instance, win32State.windowClassName);

    // Perform application initialization:
    if (!InitInstance(win32State))
    {
        result = -1;
    }
    else
    {
        win32State.acceleratorTable = ::LoadAccelerators(win32State.instance, MAKEINTRESOURCE(IDC_WIN32APP));
        if (win32State.acceleratorTable)
        {
            globalRunning = true;

            wch msg[] = L"Message";
            wch reply[64] = {};
            u32 bytesRead;
            try
            {
                ipcTransaction(SERVICE_PIPE_NAME,
                               Win32PipeHelloMsg.type,
                               msg,
                               sizeof msg,
                               reply,
                               sizeof reply,
                               &bytesRead);
            }
            catch (std::exception &e)
            {
                // Display the exception
                ::MessageBoxA(0, e.what(), "Pipe Exception", MB_OK);
            }

            // Main message loop:
            while (globalRunning)
            {
                win32ProcessPendingMessages(&win32State);
                Sleep(250);
            }
        }
        else
        {
            OutputDebugStringA("Failed to load accelerators.\n");
            result = static_cast<int>(::GetLastError());
        }
    }

    return result;
}
