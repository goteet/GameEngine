#pragma once
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicDeviceVulkan;

    struct DeviceContextVulkan : public DeferredContext, public BaseDeviceResourceVulkan
    {
        DeviceContextVulkan(GraphicDeviceVulkan* belongsTo, CommandQueueVulkan cmdQueue);
        virtual ~DeviceContextVulkan();
        virtual void Release() override;

        virtual void SetViewport(const ViewportInfo&) override { };
        virtual void SetViewports(uint32_t count, const ViewportInfo*) override { };
        virtual void SetGraphicPipelineState(GraphicPipelineState*) override { };
        virtual void SetStencilReferenceValue(uint32_t) override { };
        virtual void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, UniformBuffer**) override { };
        virtual void SetGraphicShaderResources(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, ShaderResourceView**) override { };
        virtual void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage, uint32_t startSlot, uint32_t count, SamplerState**) override { };
        virtual void SetVertexBuffers(uint32_t startSlot, uint32_t bufferBindingCount, const VertexBufferBinding* bufferBindings) override { };
        virtual void SetIndexBuffer(IndexBuffer*, uint32_t offset) override { };
        virtual void SetRenderTargets(uint32_t count, RenderTargetView** rts, DepthStencilView* ds) override { };
        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) override { };
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexOffset) override { };
        virtual void ClearRenderTarget(RenderTargetView* renderTarget, const float clearColor[4]) override { };
        virtual void ClearDepthBuffer(DepthStencilView* ds, float depth) override { };
        virtual void ClearStencilBuffer(DepthStencilView* ds, uint8_t stencil) override { };
        virtual void ClearDepthStencil(DepthStencilView* ds, float depth, uint8_t stencil) override { };
        virtual void ClearDepthStencil(DepthStencilView* ds, bool d, bool s, float dv, uint8_t sv) override { };
        virtual MappedBuffer MapBuffer(Buffer* buffer, EMapMethod mapMethod) override { return MappedBuffer(); };
        virtual void UnmapBuffer(Buffer* buffer) override { };
        virtual void UpdateBuffer(Buffer*, const void* data) override { };

        virtual void            BeginRecordCommands(const RenderingInfo&) override;
        virtual CommandQueue*   EndRecordCommands(bool bRestoreToDefaultState) override;
    private:
        CommandQueueVulkan mCommandQueue;


        VkCommandBuffer mActiveCommandBuffer = VK_NULL_HANDLE;
    };
}
