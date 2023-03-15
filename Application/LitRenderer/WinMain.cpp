#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <Foundation/Base/MemoryHelper.h>
#include "TaskGraph.h"
#include "LitRenderer.h"


const int BitmapCanvasWidth = 800;
const int BitmapCanvasHeight = 800;
const int ColorDepth = 24;
const int BitmapCanvasLinePitch = (BitmapCanvasWidth * ColorDepth + 31) / 32 * 4;
const wchar_t* AppClassName = L"SoftRendererWindowClass";
const wchar_t* AppWindowName = L"Soft Renderer";
HWND hWindow;
HDC	hdcBitmapDC = NULL;
HBITMAP hCanvasDIB = NULL;
LitRenderer* Renderer = nullptr;
BOOL WindowRefresh = false;


ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void CenterWindow(HWND, int, int);
bool Initialize(HWND);

void Uninitialize(HWND)
{
    Task::StopSystem();
    ::DeleteDC(hdcBitmapDC);
    ::DeleteObject(hCanvasDIB);
    hdcBitmapDC = NULL;
    hCanvasDIB = NULL;

    SafeDelete(Renderer);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ wchar_t* lpCmdLine,
    _In_ int		nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow) || !Initialize(hWindow))
    {
        return FALSE;
    }
    else
    {
        CenterWindow(hWindow, BitmapCanvasWidth, BitmapCanvasHeight);
    }

    MSG msg;
    bool running = true;
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

        if (WindowRefresh)
        {
            if (Renderer->GenerateImageProgressive())
            {
                WindowRefresh = false;
            }
        }

        if (Renderer->NeedUpdate())
        {
            ::RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
        }
        else
        {
            ::Sleep(16);
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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hWindow = CreateWindow(AppClassName, AppWindowName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        BitmapCanvasWidth, BitmapCanvasHeight,
        NULL, NULL, hInstance, NULL);

    if (!hWindow)
    {
        return FALSE;
    }

    ShowWindow(hWindow, nCmdShow);
    UpdateWindow(hWindow);

    return TRUE;
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
        HDC hdcWindowDC = ::BeginPaint(hWnd, &ps);
        ::BitBlt(hdcWindowDC,
            0, 0,
            BitmapCanvasWidth, BitmapCanvasHeight,
            hdcBitmapDC,
            0, 0, SRCCOPY);
        ::EndPaint(hWnd, &ps);
        WindowRefresh = true;
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CenterWindow(HWND hWnd, int canvasWidth, int canvasHeight)
{
    RECT clientRect;
    RECT windowRect;
    ::GetClientRect(hWnd, &clientRect);
    ::GetWindowRect(hWnd, &windowRect);

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);

    int screenCenterX = (mi.rcWork.right + mi.rcWork.left) / 2;
    int screenCenterY = (mi.rcWork.top + mi.rcWork.bottom) / 2;
    int borderWidth = (windowRect.right - windowRect.left - clientRect.right) / 2;
    int borderHeight = (windowRect.bottom - windowRect.top - clientRect.bottom) / 2;
    int halfWindowWidth = canvasWidth / 2 + borderWidth;
    int halfWindowHeight = canvasHeight / 2 + borderHeight;

    ::SetWindowPos(hWnd, NULL,
        screenCenterX - halfWindowWidth, screenCenterY - halfWindowHeight,
        canvasWidth + 2 * borderWidth, canvasHeight + 2 * borderHeight,
        SWP_SHOWWINDOW);
}

HBITMAP CreateDIB(HDC hdcWindowDC, int canvasWidth, int canvasHeight, VOID** canvasDataPtr)
{
    if (canvasWidth == 0)
    {
        canvasWidth = BitmapCanvasWidth;
    }

    if (canvasHeight == 0)
    {
        canvasHeight = BitmapCanvasHeight;
    }

    BITMAPINFO bmpInfo;
    ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = canvasWidth;
    bmpInfo.bmiHeader.biHeight = canvasHeight;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = ColorDepth;
    bmpInfo.bmiHeader.biCompression = BI_RGB;


    HBITMAP hBitmap = ::CreateDIBSection(hdcWindowDC, &bmpInfo, DIB_RGB_COLORS, canvasDataPtr, NULL, 0);
    return hBitmap;
}

#include <ctime>
bool Initialize(HWND hWindow)
{
    srand(static_cast<unsigned>(time(0)));

    Task::StartSystem(7);

    HDC hdcWindowDC = ::GetDC(hWindow);
    unsigned char* canvasDIBDataPtr = nullptr;
    hCanvasDIB = CreateDIB(hdcWindowDC, BitmapCanvasWidth, BitmapCanvasHeight, (VOID**)&canvasDIBDataPtr);

    if (!hCanvasDIB)
        return true;

    hdcBitmapDC = CreateCompatibleDC(hdcWindowDC);
    ::SelectObject(hdcBitmapDC, hCanvasDIB);
    ::ReleaseDC(hWindow, hdcWindowDC);

    Renderer = new LitRenderer(canvasDIBDataPtr, BitmapCanvasWidth, BitmapCanvasHeight, BitmapCanvasLinePitch);
    Renderer->Initialize();
    Renderer->GenerateImageProgressive();
    return true;
}
