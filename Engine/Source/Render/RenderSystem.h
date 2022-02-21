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

    enum EGfxIntializationError
    {
        NoError = 0,
        DeviceCreationFail,
        CreateSwapchainFail,
        CreateBackbufferRTVFail,
        CreateDeferredContextFail,
        RetrieveDXGIFactoryFail,
        RetrieveBackbufferFail,
    };

    class RenderSystem : public GE::RenderSystem
    {
    public:
        DefineRTTI;

        virtual unsigned int GetWindowWidth() const override { return mWindowWidth; }

        virtual unsigned int GetWindowHeight() const override { return mWindowHeight; }

        virtual math::point3d<float> ScreenToView(const math::point3d<int>& screen) const override { return math::point3d<float>::zero(); }

        virtual math::point3d<int> ViewToScreen(const math::point3d<float>& view) const override { return math::point3d<int>::zero(); }

    public:
        RenderSystem(void* hWindow, bool fullscreen, int width, int height);
        ~RenderSystem();

        EGfxIntializationError InitializeGfxDevice();

        void RenderFrame();

        bool OnResizeWindow(void* hWindow, int width, int height);

    public:
        struct ViewConstantBufferData
        {
            math::float4x4 ViewMatrix;
            math::float4x4 InvViewMatrix;
            math::float3   CameraPositionWS;
            math::float3   LightColor;
            math::float3   LightDirection;
        };

    private:
        void RenderSimpleBox(const ViewConstantBufferData& data);

        HWND mMainWindowHandle;
        bool mIsFullScreen = false;
        int mWindowWidth = 0;
        int mWindowHeight = 0;
        int mClientWidth = 0;
        int mClientHeight = 0;

        ComPtr<ID3D11Device> mGfxDevice = nullptr;
        ComPtr<ID3D11DeviceContext> mGfxDeviceImmediateContext = nullptr;
        ComPtr<ID3D11DeviceContext> mGfxDeviceDeferredContext = nullptr;
        ComPtr<IDXGISwapChain1> mGfxSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> mBackbuffer = nullptr;
        ComPtr<ID3D11RenderTargetView> mBackbufferRTV = nullptr;
    };
}
