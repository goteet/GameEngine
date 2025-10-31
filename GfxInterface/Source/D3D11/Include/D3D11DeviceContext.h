#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ModuleD3D11;
    struct SamplerStateD3D11;

    struct BaseContextWrapper
    {
        BaseContextWrapper(ID3D11DeviceContext* Context);
        void Release() { mD3D11Context.Reset(); }
        void SetViewport(const ViewportInfo& viewport);
        void SetViewports(unsigned int count, const ViewportInfo* viewports);
        void SetGraphicPipelineState(GraphicPipelineState* pso);
        void SetStencilReferenceValue(unsigned int value);
        void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, UniformBuffer** buffers);
        void SetGraphicShaderResources(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, ShaderResourceView** srViews);
        void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, SamplerState** states);
        void SetVertexBuffers(unsigned int startSlot, unsigned int bufferBindingCount, const DeviceContext::VertexBufferBinding* bufferBindings);
        void SetIndexBuffer(IndexBuffer* buffer, unsigned int offset);
        void SetRenderTargets(unsigned int count, RenderTargetView** renderTargetViews, DepthStencilView* depthStencilView);
        void Draw(unsigned int vertexCount, unsigned int startVertexLocation);
        void DrawIndexed(unsigned int indexCount, unsigned int startIndexLocation, unsigned int baseVertexOffset);
        void ClearRenderTarget(RenderTargetView* renderTargetView, const float clearColor[4]);
        void ClearDepthBuffer(DepthStencilView* depthStencilView, float depth);
        void ClearStencilBuffer(DepthStencilView* depthStencilView, unsigned char stencil);
        void ClearDepthStencil(DepthStencilView* depthStencilView, float depth, unsigned char stencil);
        void ClearDepthStencil(DepthStencilView* depthStencilView, bool d, bool s, float dv, unsigned char sv);
        DeviceContext::MappedBuffer MapBuffer(Buffer* buffer, DeviceContext::EMapMethod mapMethod);
        void UnmapBuffer(Buffer* buffer);
        void UpdateSubresource(Buffer* buffer, const void* sourceDataPtr);
    public:
        ComPtr<ID3D11DeviceContext> mD3D11Context;

    private:
        D3D11_MAPPED_SUBRESOURCE MapResource(ID3D11Resource* resource, DeviceContext::EMapMethod mapMethod, unsigned int subresourceIndex, bool doNotWaitGPU = false);
        void UnmapResource(ID3D11Resource* resource, unsigned int subresourceIndex);

        //TODO: concurrency-proctect
        struct RenderTargetViewCache
        {
            RenderTargetViewCache()
            {
                memset(mRenderTargetViews, 0, sizeof(mRenderTargetViews));
            }
            ID3D11RenderTargetView* mRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        };
        /*static*/ RenderTargetViewCache mRenderTargetCache;
        /*static*/ unsigned int mStencilReferenceValue = 0;

    };

    struct ImmediateContextD3D11 : public ImmediateContext
    {
        ImmediateContextD3D11(ID3D11DeviceContext* context) : mContextWrapper(context) { }
        virtual void Release() override {}

        virtual void SetViewport(const ViewportInfo& viewport) override { mContextWrapper.SetViewport(viewport); }
        virtual void SetViewports(unsigned int count, const ViewportInfo* viewports) override { mContextWrapper.SetViewports(count, viewports); }
        virtual void SetGraphicPipelineState(GraphicPipelineState* pso) override { mContextWrapper.SetGraphicPipelineState(pso); }
        virtual void SetStencilReferenceValue(unsigned int value) override { mContextWrapper.SetStencilReferenceValue(value); }
        virtual void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, UniformBuffer** buffers) override { mContextWrapper.SetGraphicUniformBuffers(stage, startSlot, count, buffers); }
        virtual void SetGraphicShaderResources(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, ShaderResourceView** srViews) override { mContextWrapper.SetGraphicShaderResources(stage, startSlot, count, srViews); }
        virtual void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, SamplerState** states) override { mContextWrapper.SetGraphicSamplerStates(stage, startSlot, count, states); }
        virtual void SetVertexBuffers(unsigned int startSlot, unsigned int bufferBindingCount, const VertexBufferBinding* bufferBindings) override { mContextWrapper.SetVertexBuffers(startSlot, bufferBindingCount, bufferBindings); }
        virtual void SetIndexBuffer(IndexBuffer* buffer, unsigned int offset) override { mContextWrapper.SetIndexBuffer(buffer, offset); }
        virtual void SetRenderTargets(unsigned int count, RenderTargetView** renderTargetView, DepthStencilView* depthStencilView) override { mContextWrapper.SetRenderTargets(count, renderTargetView, depthStencilView); }
        virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation) override { mContextWrapper.Draw(vertexCount, startVertexLocation); }
        virtual void DrawIndexed(unsigned int indexCount, unsigned int startIndexLocation, unsigned int baseVertexOffset) override { mContextWrapper.DrawIndexed(indexCount, startIndexLocation, baseVertexOffset); }
        virtual void ClearDepthBuffer(DepthStencilView* depthStencilView, float depth) override { mContextWrapper.ClearDepthBuffer(depthStencilView, depth); }
        virtual void ClearRenderTarget(RenderTargetView* renderTargetView, const float clearColor[4]) override { mContextWrapper.ClearRenderTarget(renderTargetView, clearColor); }
        virtual void ClearStencilBuffer(DepthStencilView* depthStencilView, unsigned char stencil) override { mContextWrapper.ClearStencilBuffer(depthStencilView, stencil); }
        virtual void ClearDepthStencil(DepthStencilView* depthStencilView, float depth, unsigned char stencil) override { mContextWrapper.ClearDepthStencil(depthStencilView, depth, stencil); }
        virtual void ClearDepthStencil(DepthStencilView* depthStencilView, bool d, bool s, float dv, unsigned char sv) override { mContextWrapper.ClearDepthStencil(depthStencilView, d, s, dv, sv); }
        virtual void ExecuteCommandQueue(CommandQueue*, bool bRestoreToDefaultState) override;
        virtual MappedBuffer MapBuffer(Buffer* buffer, EMapMethod mapMethod) override { return mContextWrapper.MapBuffer(buffer, mapMethod); }
        virtual void UnmapBuffer(Buffer* buffer)  override { mContextWrapper.UnmapBuffer(buffer); }
        virtual void UpdateBuffer(Buffer* buffer, const void* data) override { mContextWrapper.UpdateSubresource(buffer, data); }
    private:
        BaseContextWrapper mContextWrapper;
    };

    struct DeferredContextD3D11 : public DeferredContext
    {
        DeferredContextD3D11(ID3D11DeviceContext* context) : mContextWrapper(context) { }
        virtual void Release() override {}

        virtual void SetViewport(const ViewportInfo& viewport) override { mContextWrapper.SetViewport(viewport); }
        virtual void SetViewports(unsigned int count, const ViewportInfo* viewports) override { mContextWrapper.SetViewports(count, viewports); }
        virtual void SetGraphicPipelineState(GraphicPipelineState* pso) override { mContextWrapper.SetGraphicPipelineState(pso); }
        virtual void SetStencilReferenceValue(unsigned int value) override { mContextWrapper.SetStencilReferenceValue(value); }
        virtual void SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, UniformBuffer** buffers) override { mContextWrapper.SetGraphicUniformBuffers(stage, startSlot, count, buffers); }
        virtual void SetGraphicShaderResources(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, ShaderResourceView** srViews) override { mContextWrapper.SetGraphicShaderResources(stage, startSlot, count, srViews); }
        virtual void SetGraphicSamplerStates(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, SamplerState** states) override { mContextWrapper.SetGraphicSamplerStates(stage, startSlot, count, states); }
        virtual void SetVertexBuffers(unsigned int startSlot, unsigned int bufferBindingCount, const VertexBufferBinding* bufferBindings) override { mContextWrapper.SetVertexBuffers(startSlot, bufferBindingCount, bufferBindings); }
        virtual void SetIndexBuffer(IndexBuffer* buffer, unsigned int offset) override { mContextWrapper.SetIndexBuffer(buffer, offset); }
        virtual void SetRenderTargets(unsigned int count, RenderTargetView** renderTargetViews, DepthStencilView* depthStencilView) override { mContextWrapper.SetRenderTargets(count, renderTargetViews, depthStencilView); }
        virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation) override { mContextWrapper.Draw(vertexCount, startVertexLocation); }
        virtual void DrawIndexed(unsigned int indexCount, unsigned int startIndexLocation, unsigned int baseVertexOffset) override { mContextWrapper.DrawIndexed(indexCount, startIndexLocation, baseVertexOffset); }
        virtual void ClearDepthBuffer(DepthStencilView* depthStencilView, float depth) override { mContextWrapper.ClearDepthBuffer(depthStencilView, depth); }
        virtual void ClearRenderTarget(RenderTargetView* renderTargetView, const float clearColor[4]) override { mContextWrapper.ClearRenderTarget(renderTargetView, clearColor); }
        virtual void ClearStencilBuffer(DepthStencilView* depthStencilView, unsigned char stencil) override { mContextWrapper.ClearStencilBuffer(depthStencilView, stencil); }
        virtual void ClearDepthStencil(DepthStencilView* depthStencilView, float depth, unsigned char stencil) override { mContextWrapper.ClearDepthStencil(depthStencilView, depth, stencil); }
        virtual void ClearDepthStencil(DepthStencilView* depthStencilView, bool d, bool s, float dv, unsigned char sv) override { mContextWrapper.ClearDepthStencil(depthStencilView, d, s, dv, sv); }
        virtual MappedBuffer MapBuffer(Buffer* buffer, EMapMethod mapMethod) override { return mContextWrapper.MapBuffer(buffer, mapMethod); }
        virtual void UnmapBuffer(Buffer* buffer)  override { mContextWrapper.UnmapBuffer(buffer); }
        virtual void UpdateBuffer(Buffer* buffer, const void* data) override { mContextWrapper.UpdateSubresource(buffer, data); }

        virtual void            BeginRecordCommands(const RenderingInfo&) override {}
        virtual CommandQueue*   EndRecordCommands(bool bRestoreToDefaultState) override;
    private:
        BaseContextWrapper mContextWrapper;
    };
}
