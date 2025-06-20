#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicModuleVulkan;

    struct GraphicDeviceVulkan : public GraphicDevice
    {
        GraphicDeviceVulkan(GraphicModuleVulkan* BelongsTo, VkPhysicalDevice VulkanPhysicalDevice, VkDevice VulkanDevice,
            uint32_t GraphicQueueFamilyIndex, uint32_t GraphicQueueIndex,
            uint32_t ComputeQueueFamilyIndex, uint32_t ComputeQueueIndex,
            uint32_t TransferQueueFamilyIndex, uint32_t TransferQueueIndex);
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
        VkPhysicalDevice mVulkanPhysicalDevice;
        VkDevice mVulkanDevice;
        VkQueue  mVulkanGraphicQueue;
        VkQueue  mVulkanComputeQueue;
        VkQueue  mVulkanTransferQueue;

        uint32_t mGraphicQueueFamilyIndex,  mGraphicQueueIndex;
        uint32_t mComputeQueueFamilyIndex,  mComputeQueueIndex;
        uint32_t mTransferQueueFamilyIndex, mTransferQueueIndex;
    };
}
