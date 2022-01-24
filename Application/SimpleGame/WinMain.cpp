#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <GEInclude.h>
#include "SimpleTimer.h"

const wchar_t* AppClassName = L"SimpleGameWindowClass";
const wchar_t* AppWindowName = L"Simple Game";
HWND hWindow;
GE::GameEngine* g_GameEngine;

bool Initialize(HINSTANCE hInstance, int nCmdShow)
{
    int windowWidth = 1280;
    int windowHeight = 720;

    HWND hWindow = CreateWindow(AppClassName, AppWindowName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        windowWidth, windowHeight, NULL, NULL, hInstance, NULL);

    if (hWindow)
    {
        ShowWindow(hWindow, nCmdShow);
        UpdateWindow(hWindow);

        GE::GameEngine::CreationConfig Config;
        Config.NativeWindow = hWindow;
        Config.AbsoluteResourceFolderPath = nullptr;
        Config.IsFullScreen = false;
        Config.InitialWidth = windowWidth;
        Config.InitialHeight = windowHeight;

        using GE::GameEngine;
        GameEngine::InitializeResult result = GE::GameEngine::Initialize(Config, g_GameEngine);

        return result == GameEngine::InitializeResult::Success;
    }
    return false;
}

bool Uninitialize(HWND)
{
    GE::GameEngine::Uninitialize();
    return true;
}

bool NeedUpdate() { return false; }

bool Present(HDC) { return true; }


ATOM MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ wchar_t*       lpCmdLine,
    _In_ int            nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);
    if (!Initialize(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    bool running = true;
    SimpleTimer mainLoopTimer;
    while (running)
    {
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
        }

        int64_t deltaMilliseconds = mainLoopTimer.ElapsedMilliseconds();
        mainLoopTimer.Record();
        g_GameEngine->Update((unsigned int)deltaMilliseconds);

        if (NeedUpdate())
        {
            ::RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
        }
    }

    Uninitialize(hWindow);
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = AppClassName;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(hWnd, &ps);
        Present(hdc);
        ::EndPaint(hWnd, &ps);
    }
    break;
    case WM_SIZE:
    {
        if (g_GameEngine != nullptr)
        {
            int clientWidth = LOWORD(lParam);
            int clientHeight = HIWORD(lParam);
            g_GameEngine->OnResizeWindow(hWnd, clientWidth, clientHeight);
        }
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
