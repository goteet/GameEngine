#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicModuleVulkan;

    struct GraphicDeviceVulkan : public GraphicDevice
    {
        GraphicDeviceVulkan(GraphicModuleVulkan* belongsTo, VkDevice);
        virtual ~GraphicDeviceVulkan();
        virtual void Release() override;

        virtual SwapChain*              CreateSwapChain(void* windowHandle, int windowWidth, int windowHeight, bool isFullscreen) override;
        virtual GraphicPipelineState*   CreateGraphicPipelineState(const GraphicPipelineState::CreateInfo&) override { return nullptr; }
        virtual ComputePipelineState*   CreateComputePipelineState(const ComputePipelineState::CreateInfo&) override { return nullptr; }
        virtual SamplerState*           CreateSamplerState(const SamplerState::CreateInfo&) override { return nullptr; }
        virtual ShaderBinary*           CompileShader(const ShaderBinary::CreateInfo&) override { return nullptr; }
        virtual Shader*                 CreateShader(const Shader::CreateInfo&) override { return nullptr; }
        virtual VertexBuffer*           CreateVertexBuffer(const VertexBuffer::CreateInfo&) override { return nullptr; }
        virtual IndexBuffer*            CreateIndexBuffer(const IndexBuffer::CreateInfo&) override { return nullptr; }
        virtual UniformBuffer*          CreateUniformbuffer(const UniformBuffer::CreateInfo&) override { return nullptr; }
        virtual RenderTargetView*       CreateRenderTargetView(const RenderTargetView::CreateInfo&) override { return nullptr; }
        virtual DepthStencilView*       CreateDepthStencilView(const DepthStencilView::CreateInfo&) override { return nullptr; }
        virtual ImmediateContext*       GetImmediateContext() override { return nullptr; }
        virtual DeferredContext*        GetDeferredContext()  override { return nullptr; }
    private:
        GraphicModuleVulkan* mBelongsTo;
        VkDevice mVulkanDevice;
        VkQueue  mVulkanQueue;
    };
}
