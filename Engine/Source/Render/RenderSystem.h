#pragma once
#include <memory>
#include <GfxInterface.h>
#include "PreIncludeFiles.h"
#include "TransientBufferRegistry.h"

namespace engine
{
    using namespace Microsoft::WRL;

    class Scene;

    struct VertexLayout
    {
        math::float4 Position;
        math::float3 Normal;
        math::float2 Texcoord;
    };


    enum class EGfxIntializationError
    {
        NoError = 0,
        DeviceCreationFail,
        CreateSwapchainFail,
        CreateBackbufferDSFail,
        CreateBackbufferRTVFail,
        CreateBackbufferSRVFail,
        CreateDeferredContextFail,
        RetrieveDXGIFactoryFail,
        RetrieveBackbufferFail,
    };

    class RenderSystem : public GE::RenderSystem
    {
    public:
        DefineRTTI;

        virtual GFXI::GraphicDevice* GetGfxDevice() override { return mGfxDevice; }
        //virtual GE::GfxDeviceImmediateContext* GetGfxDeviceImmediateContext() override { return mGfxDeviceImmediateContext.get(); }
        virtual unsigned int GetWindowWidth() const override { return mWindowWidth; }
        virtual unsigned int GetWindowHeight() const override { return mWindowHeight; }
        virtual math::point3d<float> ScreenToView(const math::point3d<int>& screen) const override { return math::point3d<float>::zero(); }
        virtual math::point3d<int> ViewToScreen(const math::point3d<float>& view) const override { return math::point3d<int>::zero(); }
        virtual void SetRenderingWorldMatrixForTest(const math::float4x4& matrix) override { SetObjectToWorldMatrixForTest(matrix); }
        virtual void FillEntireEntireBufferFromMemory(GFXI::Buffer*, const void* data) override;

    public:
        RenderSystem(void* hWindow, bool fullscreen, int width, int height);
        ~RenderSystem();
        EGfxIntializationError InitializeGfxDevice();
        void RenderFrame(Scene& scene);
        bool OnResizeWindow(void* hWindow, int width, int height);
        void SetObjectToWorldMatrixForTest(const math::float4x4& matrix);
        TransientBufferRegistry* GetTransientBufferRegistry() { return mTransientBufferRegistry.get(); }

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
        void RenderScene(Scene& scene, GFXI::DeferredContext& context, const ViewConstantBufferData& data, bool shadow);

        HWND mMainWindowHandle;
        bool mIsFullScreen = false;
        int mWindowWidth = 0;
        int mWindowHeight = 0;
        int mClientWidth = 0;
        int mClientHeight = 0;
        std::unique_ptr<TransientBufferRegistry> mTransientBufferRegistry = nullptr;

        GFXI::GraphicModule* mGfxModule = nullptr;
        GFXI::GraphicDevice* mGfxDevice = nullptr;
        GFXI::SwapChain* mMainWindowSwapChain = nullptr;
        GFXI::DepthStencilView* mMainWindowDepthStencil = nullptr;

        GFXI::SamplerState* mDefaultSamplerState = nullptr;

        GFXI::CommandQueue* mCubeRenderingQueueShadow = nullptr;
        GFXI::CommandQueue* mCubeRenderingQueueFinal = nullptr;
        GFXI::CommandQueue* mBlitRenderingQueue = nullptr;
        
        GFXI::Shader* mShadowPassVS       = nullptr;
        GFXI::Shader* mShadowPassPS       = nullptr;
        GFXI::Shader* mSimpleDrawPassVS   = nullptr;
        GFXI::Shader* mSimpleDrawPassPS   = nullptr;
        GFXI::Shader* mBlitPassVS         = nullptr;
        GFXI::Shader* mBlitPassPS         = nullptr;
        GFXI::GraphicPipelineState* mShadowPassState     = nullptr;
        GFXI::GraphicPipelineState* mSimpleDrawPassState = nullptr;
        GFXI::GraphicPipelineState* mBlitPassState       = nullptr;

        GFXI::VertexBuffer* mFullsceenQuadVerticePositions = nullptr;
        GFXI::VertexBuffer* mFullsceenQuadVertices = nullptr;

        GFXI::UniformBuffer* mSceneUniformBuffer = nullptr;
        GFXI::UniformBuffer* mObjectUniformBuffer = nullptr;
        GFXI::UniformBuffer* mLightViewUniformBuffer = nullptr;

    };
}
