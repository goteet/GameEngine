#pragma once
//we are using DX11 directly now, may abstract GfxDevice when we are ready.
#include <Windows.h>
#include <d3d11_3.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include "GEInclude.h"

namespace engine
{
    using namespace Microsoft::WRL;
    class RenderSystem : public GE::RenderSystem
    {
    public:
        DefineRTTI;

        virtual unsigned int GetWindowWidth() const override { return mWindowWidth; }

        virtual unsigned int GetWindowHeight() const override { return mWindowHeight; }

        virtual math::point3d<float> ScreenToView(const math::point3d<int>& screen) const override { return math::point3d<float>::zero(); }

        virtual math::point3d<int> ViewToScreen(const math::point3d<float>& view) const override { return math::point3d<int>::zero(); }

        RenderSystem(void* hWindow, bool fullscreen, int width, int height);

        bool InitializeGfxDevice();

        void RenderFrame();

        bool OnResizeWindow(void* hWindow, int width, int height);

    private:
        HWND mMainWindowHandle;
        bool mIsFullScreen = false;
        int mWindowWidth = 0;
        int mWindowHeight = 0;
        int mClientWidth = 0;
        int mClientHeight = 0;

        ComPtr<ID3D11Device> mGfxDevice = nullptr;
        ComPtr<ID3D11DeviceContext> mGfxDeviceContext = nullptr;
        ComPtr<IDXGISwapChain1> mGfxSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> mBackbuffer = nullptr;
        ComPtr<ID3D11RenderTargetView> mBackbufferRTV = nullptr;
    };
}
