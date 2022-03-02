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
GE::Scene* g_DefaultScene;

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

void InitializeSimpleScene()
{
    g_DefaultScene = g_GameEngine->CreateOrGetDefaultScene();

    auto nodeCube = g_DefaultScene->CreateSceneNode();
    nodeCube->SetLocalScale(math::float3(1.2f, 2, 1.2f));
    nodeCube->SetRightDirection(math::float3(1, 0, 0.2f));
    nodeCube->SetLocalPosition(math::point3df(1.0f, -1.2f, 1.25f));
    GE::MeshRenderer* boxMesh = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Box); nodeCube->AddComponent(boxMesh, GE::AutoReleaseComponent);

    auto nodeLWall = g_DefaultScene->CreateSceneNode();
    auto nodeRWall = g_DefaultScene->CreateSceneNode();
    auto nodeFWall = g_DefaultScene->CreateSceneNode();
    auto nodeTWall = g_DefaultScene->CreateSceneNode();
    auto nodeBWall = g_DefaultScene->CreateSceneNode();

    GE::MeshRenderer* planeMeshL = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Plane);    nodeLWall->AddComponent(planeMeshL, GE::AutoReleaseComponent);
    GE::MeshRenderer* planeMeshR = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Plane);    nodeRWall->AddComponent(planeMeshR, GE::AutoReleaseComponent);
    GE::MeshRenderer* planeMeshF = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Plane);    nodeFWall->AddComponent(planeMeshF, GE::AutoReleaseComponent);
    GE::MeshRenderer* planeMeshT = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Plane);    nodeTWall->AddComponent(planeMeshT, GE::AutoReleaseComponent);
    GE::MeshRenderer* planeMeshB = GE::MeshRenderer::CreateMeshRenderer(GE::MeshRenderer::EMeshType::Plane);    nodeBWall->AddComponent(planeMeshB, GE::AutoReleaseComponent);

    nodeLWall->SetUpDirection(math::normalized_float3::unit_x());
    nodeRWall->SetUpDirection(math::normalized_float3::unit_x_neg());
    nodeFWall->SetUpDirection(math::normalized_float3::unit_z_neg());
    nodeTWall->SetUpDirection(math::normalized_float3::unit_y_neg());
    nodeBWall->SetUpDirection(math::normalized_float3::unit_y());

    nodeLWall->SetLocalScale(math::float3(5.0f, 5.0f, 5.0f));
    nodeRWall->SetLocalScale(math::float3(5.0f, 5.0f, 5.0f));
    nodeFWall->SetLocalScale(math::float3(5.0f, 5.0f, 5.0f));
    nodeTWall->SetLocalScale(math::float3(5.0f, 5.0f, 5.0f));
    nodeBWall->SetLocalScale(math::float3(5.0f, 5.0f, 5.0f));

    nodeLWall->SetLocalPosition(math::point3df(-2.5, 0, 0));
    nodeRWall->SetLocalPosition(math::point3df(+2.5, 0, 0));
    nodeFWall->SetLocalPosition(math::point3df(0, 0, +2.5));
    nodeTWall->SetLocalPosition(math::point3df(0, +2.5, 0));
    nodeBWall->SetLocalPosition(math::point3df(0, -2.5, 0));

    GE::Camera* mainCamera = g_DefaultScene->GetDefaultCamera();
    mainCamera->SetEyePosition(math::point3df(0, 5, -20));
    mainCamera->Lookat(math::point3df(0, 0, 0));

    GE::DirectionalLight* mainLight = g_DefaultScene->CreateDirectionalLightNode();
    mainLight->GetSceneNode()->SetWorldPosition(math::point3df(5.0f, 5.0f, -5.0f));
    math::normalized_float3 lightDirection(0.7f, -0.8f, 1.0f);
    mainLight->GetSceneNode()->SetForwardDirection(lightDirection);
    mainLight->SetIntensity(1.2f);
    mainLight->SetColor(math::float3(0.98f, 0.98f, 0.9f));

}

bool NeedUpdate() { return false; }

bool Present(HDC) { return true; }


ATOM MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE  hPrevInstance,
    _In_ wchar_t* lpCmdLine,
    _In_ int            nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);
    if (!Initialize(hInstance, nCmdShow))
    {
        return FALSE;
    }

    InitializeSimpleScene();

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
