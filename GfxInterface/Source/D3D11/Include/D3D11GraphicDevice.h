#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"
#include "D3D11DeviceContext.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct GraphicModuleD3D11;

    struct GraphicDeviceD3D11 : public GraphicDevice
    {
        GraphicDeviceD3D11(GraphicModuleD3D11* BelongsTo, ID3D11Device*, ID3D11DeviceContext* Immediate, ID3D11DeviceContext* Deferred);
        virtual ~GraphicDeviceD3D11();
        virtual void Release() override;

        virtual SwapChain*              CreateSwapChain(void* WindowHandle, int32_t WindowWidth, int32_t WindowHeight, bool IsFullscreen) override;
        virtual GraphicPipelineState*   CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo&) override;
        virtual ComputePipelineState*   CreateComputePipelineState(const ComputePipelineState::CreateInfo&) override { return nullptr; }
        virtual SamplerState*           CreateSamplerState(const SamplerState::CreateInfo&) override;
        virtual ShaderBinary*           CompileShader(const ShaderBinary::CreateInfo&) override;
        virtual Shader*                 CreateShader(const Shader::CreateInfo&) override;
        virtual VertexBuffer*           CreateVertexBuffer(const VertexBuffer::CreateInfo&) override;
        virtual IndexBuffer*            CreateIndexBuffer(const IndexBuffer::CreateInfo&) override;
        virtual UniformBuffer*          CreateUniformbuffer(const UniformBuffer::CreateInfo&) override;
        virtual RenderTargetView*       CreateRenderTargetView(const RenderTargetView::CreateInfo&) override;
        virtual DepthStencilView*       CreateDepthStencilView(const DepthStencilView::CreateInfo&) override;

        virtual ImmediateContext*       GetImmediateContext() override  { return &mImmediateContext; }
        virtual DeferredContext*        GetDeferredContext()  override  { return &mDeferredContext; }

    private:
        GraphicModuleD3D11*     mBelongsTo;
        ComPtr<ID3D11Device>    mD3D11Device;
        ImmediateContextD3D11   mImmediateContext;
        DeferredContextD3D11    mDeferredContext;
    };
}
