#pragma once
#include "PreIncludeFiles.h"
#include "GfxInterface.h"

namespace engine
{
    using namespace Microsoft::WRL;

    class Scene;

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
        void RenderFrame(Scene& scene);
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
        std::unique_ptr<GfxDevice> mGfxDevice = nullptr;
        std::unique_ptr<GfxImmediateContext> mGfxDeviceImmediateContext = nullptr;
        std::unique_ptr<GfxDeviceContext> mGfxDeviceDeferredContext = nullptr;
        ComPtr<IDXGISwapChain1> mGfxSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> mBackbuffer = nullptr;
        ComPtr<ID3D11RenderTargetView> mBackbufferRTV = nullptr;
    };
}
