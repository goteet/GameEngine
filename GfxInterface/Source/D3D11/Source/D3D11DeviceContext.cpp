#include "D3D11Buffer.h"
#include "D3D11CommandQueue.h"
#include "D3D11DeviceContext.h"
#include "D3D11GraphicDevice.h"
#include "D3D11PipelineState.h"
#include "D3D11SamplerState.h"
#include "D3D11SwapChain.h"
#include "D3D11Texture.h"

namespace GFXI
{
    //BaseContextWrapper::RenderTargetViewCache BaseContextWrapper::mRenderTargetCache;
    //unsigned int BaseContextWrapper::mStencilReferenceValue = 0;
    BaseContextWrapper::BaseContextWrapper(ID3D11DeviceContext* Context)
        : mD3D11Context(Context)
    {     
    }
    void BaseContextWrapper::SetViewport(const ViewportInfo& viewport)
    {
        D3D11_VIEWPORT D3D11Viewport;
        D3D11Viewport.TopLeftX = viewport.X;
        D3D11Viewport.TopLeftY = viewport.Y;
        D3D11Viewport.Width = viewport.Width;
        D3D11Viewport.Height = viewport.Height;
        D3D11Viewport.MinDepth = viewport.MinDepth;
        D3D11Viewport.MaxDepth = viewport.MaxDepth;
        mD3D11Context->RSSetViewports(1, &D3D11Viewport);
    }

    void BaseContextWrapper::SetViewports(unsigned int count, const ViewportInfo* viewports)
    {
        if (count > D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
        {
            count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        }

        D3D11_VIEWPORT D3D11Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        for (unsigned int index = 0; index < count; index++)
        {
            D3D11Viewports[index].TopLeftX = viewports[index].X;
            D3D11Viewports[index].TopLeftY = viewports[index].Y;
            D3D11Viewports[index].Width = viewports[index].Width;
            D3D11Viewports[index].Height = viewports[index].Height;
            D3D11Viewports[index].MinDepth = viewports[index].MinDepth;
            D3D11Viewports[index].MaxDepth = viewports[index].MaxDepth;
        }
        mD3D11Context->RSSetViewports(count, D3D11Viewports);
    }

    void BaseContextWrapper::SetGraphicPipelineState(GraphicPipelineState* pso)
    {
        GraphicPipelineStateD3D11* D3D11GraphicPSO = dynamic_cast<GraphicPipelineStateD3D11*>(pso);
        D3D11GraphicPSO->SetupContext(mD3D11Context.Get(), mStencilReferenceValue);
    }

    void BaseContextWrapper::SetGraphicUniformBuffers(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, UniformBuffer** buffers)
    {
        if (count > 0)
        {
            std::vector<ID3D11Buffer*> uniformBuffers(count);
            for (unsigned int index = 0; index < count; index++)
            {
                if (buffers[index])
                {
                    uniformBuffers[index] = dynamic_cast<UniformBufferD3D11*>(buffers[index])->GetD3D11Buffer();
                }
                else
                {
                    uniformBuffers[index] = nullptr;
                }
            }

            using EShaderStage = GraphicPipelineState::EShaderStage;
            switch (stage)
            {
            case EShaderStage::Vertex:      mD3D11Context->VSSetConstantBuffers(startSlot, count, uniformBuffers.data()); break;
            default:
            case EShaderStage::Pixel:       mD3D11Context->PSSetConstantBuffers(startSlot, count, uniformBuffers.data()); break;
            case EShaderStage::Geometry:    mD3D11Context->GSSetConstantBuffers(startSlot, count, uniformBuffers.data()); break;
            case EShaderStage::Domain:      mD3D11Context->DSSetConstantBuffers(startSlot, count, uniformBuffers.data()); break;
            case EShaderStage::Hull:        mD3D11Context->HSSetConstantBuffers(startSlot, count, uniformBuffers.data()); break;
            }
        }
    }

    void BaseContextWrapper::SetGraphicShaderResources(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, ShaderResourceView** srViews)
    {
        if (count > 0)
        {
            std::vector<ID3D11ShaderResourceView*> shaderResourceViews(count);
            for (unsigned int index = 0; index < count; index++)
            {
                if (srViews[index])
                {
                    shaderResourceViews[index] = dynamic_cast<ShaderResourceViewD3D11*>(srViews[index])->GetD3D11ShaderResourceView();
                }
                else
                {
                    shaderResourceViews[index] = nullptr;
                }
            }

            using EShaderStage = GraphicPipelineState::EShaderStage;
            switch (stage)
            {
            case EShaderStage::Vertex:      mD3D11Context->VSSetShaderResources(startSlot, count, shaderResourceViews.data()); break;
            default:
            case EShaderStage::Pixel:       mD3D11Context->PSSetShaderResources(startSlot, count, shaderResourceViews.data()); break;
            case EShaderStage::Geometry:    mD3D11Context->GSSetShaderResources(startSlot, count, shaderResourceViews.data()); break;
            case EShaderStage::Domain:      mD3D11Context->DSSetShaderResources(startSlot, count, shaderResourceViews.data()); break;
            case EShaderStage::Hull:        mD3D11Context->HSSetShaderResources(startSlot, count, shaderResourceViews.data()); break;
            }
        }
    }

    void BaseContextWrapper::SetGraphicSamplerStates(GraphicPipelineState::EShaderStage stage, unsigned int startSlot, unsigned int count, SamplerState** states)
    {
        if (count > 0)
        {
            std::vector<ID3D11SamplerState*> samplerStates(count);
            for (unsigned int index = 0; index < count; index++)
            {
                if (states[index])
                {
                    samplerStates[index] = dynamic_cast<SamplerStateD3D11*>(states[index])->GetD3D11SamplerState();
                }
                else
                {
                    samplerStates[index] = nullptr;
                }
            }

            using EShaderStage = GraphicPipelineState::EShaderStage;
            switch (stage)
            {
            case EShaderStage::Vertex:      mD3D11Context->VSSetSamplers(startSlot, count, samplerStates.data()); break;
            default:
            case EShaderStage::Pixel:       mD3D11Context->PSSetSamplers(startSlot, count, samplerStates.data()); break;
            case EShaderStage::Geometry:    mD3D11Context->GSSetSamplers(startSlot, count, samplerStates.data()); break;
            case EShaderStage::Domain:      mD3D11Context->DSSetSamplers(startSlot, count, samplerStates.data()); break;
            case EShaderStage::Hull:        mD3D11Context->HSSetSamplers(startSlot, count, samplerStates.data()); break;
            }
        }
    }

    void BaseContextWrapper::SetStencilReferenceValue(unsigned int value)
    {
        if (value != mStencilReferenceValue)
        {
            mStencilReferenceValue = value;

            UINT oldStencilValue;
            ID3D11DepthStencilState* depthStencilState = nullptr;
            mD3D11Context->OMGetDepthStencilState(&depthStencilState, &oldStencilValue);
            mD3D11Context->OMSetDepthStencilState(depthStencilState, mStencilReferenceValue);
            depthStencilState->Release();
        }
    }
    void BaseContextWrapper::SetVertexBuffers(unsigned int startSlot, unsigned int bufferBindingCount, const DeviceContext::VertexBufferBinding* bufferBindings)
    {
        if (bufferBindingCount > 0)
        {
            std::vector<ID3D11Buffer*> buffers(bufferBindingCount);
            std::vector<unsigned int> strides(bufferBindingCount);
            std::vector<unsigned int> offsets(bufferBindingCount);
            for (unsigned int index = 0; index < bufferBindingCount; index++)
            {
                const GFXI::DeviceContext::VertexBufferBinding& binding = bufferBindings[index];
                buffers[index] = dynamic_cast<VertexBufferD3D11*>(binding.VertexBuffer)->GetD3D11Buffer();
                strides[index] = binding.ElementStride;
                offsets[index] = binding.BufferOffset;
            }
            mD3D11Context->IASetVertexBuffers(startSlot, bufferBindingCount, buffers.data(), strides.data(), offsets.data());
        }
    }

    DXGI_FORMAT D3D11IndexBufferFormatMap[] =
    {
        DXGI_FORMAT_R32_UINT,
        DXGI_FORMAT_R16_UINT
    };

    void BaseContextWrapper::SetIndexBuffer(IndexBuffer* buffer, unsigned int offset)
    {
        ID3D11Buffer* D3D11IndexBuffer = dynamic_cast<IndexBufferD3D11*>(buffer)->GetD3D11Buffer();
        DXGI_FORMAT format = D3D11IndexBufferFormatMap[static_cast<int>(buffer->GetIndexFormat())];
        mD3D11Context->IASetIndexBuffer(D3D11IndexBuffer, format, offset);
    }

    void BaseContextWrapper::Draw(unsigned int vertexCount, unsigned int startVertexLocation)
    {
        mD3D11Context->Draw(vertexCount, startVertexLocation);
    }

    void BaseContextWrapper::DrawIndexed(unsigned int indexCount, unsigned int startIndexLocation, unsigned int baseVertexOffset)
    {
        mD3D11Context->DrawIndexed(indexCount, startIndexLocation, baseVertexOffset);
    }

    void BaseContextWrapper::ClearRenderTarget(RenderTargetView* renderTarget, const float clearColor[4])
    {
        ID3D11RenderTargetView* renderTargetView = dynamic_cast<RenderTargetD3D11*>(renderTarget)->GetRenderTargetView();
        mD3D11Context->ClearRenderTargetView(renderTargetView, clearColor);
    }

    void BaseContextWrapper::ClearDepthBuffer(DepthStencilView* ds, float depth)
    {
        ID3D11DepthStencilView* dsView = dynamic_cast<DepthStencilD3D11*>(ds)->GetDepthStencilView();
        mD3D11Context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, depth, 0x0);
    }

    void BaseContextWrapper::ClearStencilBuffer(DepthStencilView* ds, unsigned char stencil)
    {
        ID3D11DepthStencilView* dsView = dynamic_cast<DepthStencilD3D11*>(ds)->GetDepthStencilView();
        mD3D11Context->ClearDepthStencilView(dsView, D3D11_CLEAR_STENCIL, 0.0f, stencil);
    }

    void BaseContextWrapper::ClearDepthStencil(DepthStencilView* ds, float depth, unsigned char stencil)
    {
        ID3D11DepthStencilView* dsView = dynamic_cast<DepthStencilD3D11*>(ds)->GetDepthStencilView();
        mD3D11Context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void BaseContextWrapper::ClearDepthStencil(DepthStencilView* ds, bool d, bool s, float dv, unsigned char sv)
    {
        ID3D11DepthStencilView* dsView = dynamic_cast<DepthStencilD3D11*>(ds)->GetDepthStencilView();
        UINT clearFlag = (s ? D3D11_CLEAR_STENCIL : 0) | (d ? D3D11_CLEAR_DEPTH : 0);
        mD3D11Context->ClearDepthStencilView(dsView, clearFlag, dv, sv);
    }

    ID3D11Buffer* GetD3D11Buffer(GFXI::Buffer* buffer)
    {
        ID3D11Buffer* D3D11Buffer = nullptr;
        switch (buffer->GetBinding())
        {
        case Buffer::EBinding::Vertex:
            return dynamic_cast<VertexBufferD3D11*>(buffer)->GetD3D11Buffer();
            break;
        case Buffer::EBinding::Index:
            return dynamic_cast<IndexBufferD3D11*>(buffer)->GetD3D11Buffer();
            break;
        case Buffer::EBinding::Uniform:
            return dynamic_cast<UniformBufferD3D11*>(buffer)->GetD3D11Buffer();
            break;
        default:break;
        }
        return nullptr;
    }
    DeviceContext::MappedBuffer BaseContextWrapper::MapBuffer(Buffer* buffer, DeviceContext::EMapMethod mapMethod)
    {
        ID3D11Buffer* D3D11Buffer = GetD3D11Buffer(buffer);

        if (D3D11Buffer)
        {
            D3D11_MAPPED_SUBRESOURCE mapped = MapResource(D3D11Buffer, mapMethod, 0);
            return DeviceContext::MappedBuffer{ mapped.pData, mapped.RowPitch };
        }
        else
        {
            return DeviceContext::MappedBuffer{ nullptr, 0 };
        }
    }

    void BaseContextWrapper::UnmapBuffer(Buffer* buffer)
    {
        ID3D11Buffer* D3D11Buffer = GetD3D11Buffer(buffer);

        if (D3D11Buffer)
        {
            UnmapResource(D3D11Buffer, 0);
        }
        else
        {
            //assert(0);
        }
    }

    void BaseContextWrapper::UpdateSubresource(Buffer* buffer, const void* sourceDataPtr)
    {
        if (buffer->GetUsage() == EDataUsage::Immutable || buffer->GetUsage() == EDataUsage::Dynamic)
        {
            return;
        }
        ID3D11Buffer* D3D11Buffer = GetD3D11Buffer(buffer);
        UINT subResource = 0;
        UINT sourceRowPitch = buffer->GetBufferSize();
        UINT sourceDepthPitch = 0;
        mD3D11Context->UpdateSubresource(D3D11Buffer, subResource, nullptr,
            sourceDataPtr, sourceRowPitch, sourceDepthPitch);
    }

    D3D11_MAP D3D11MapMethodMap[] =
    {
        D3D11_MAP::D3D11_MAP_READ, //dummy.
        D3D11_MAP::D3D11_MAP_READ,
        D3D11_MAP::D3D11_MAP_WRITE,
        D3D11_MAP::D3D11_MAP_READ_WRITE,
        D3D11_MAP::D3D11_MAP_WRITE_DISCARD,
        D3D11_MAP::D3D11_MAP_WRITE_NO_OVERWRITE
    };

    D3D11_MAPPED_SUBRESOURCE BaseContextWrapper::MapResource(ID3D11Resource* resource, DeviceContext::EMapMethod mapMethod, unsigned int subresourceIndex, bool doNotWaitGPU)
    {
        //Flag that specifies what the CPU does when the GPU is busy
        // The only value is : D3D11_MAP_FLAG_DO_NOT_WAIT
        // cannot be used with D3D11_MAP_WRITE_DISCARD/D3D11_MAP_WRITE_NOOVERWRITE.
        UINT mapFlags = doNotWaitGPU ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0;
        D3D11_MAPPED_SUBRESOURCE mappedResources;
        memset(&mappedResources, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

        D3D11_MAP D3D11MapMethod = D3D11MapMethodMap[static_cast<int>(mapMethod)];
        mD3D11Context->Map(resource, subresourceIndex, D3D11MapMethod, mapFlags, &mappedResources);

        /*
        * The values in these members tell you how much data you can view:
        *   pData points to row 0 and depth slice 0.
        *   RowPitch contains the value that the runtime adds to pData to move from row to row, where each row contains multiple pixels.
        *   DepthPitch contains the value that the runtime adds to pData to move from depth slice to depth slice, where each depth slice contains multiple rows.
        * When RowPitch and DepthPitch are not appropriate for the resource type, the runtime might set their values to 0.
        * So, don't use these values for anything other than iterating over rows and depth. Here are some examples:
        *   For Buffer and Texture1D, the runtime assigns values that aren't 0 to RowPitch and DepthPitch.
        *       For example, if a Buffer contains 8 bytes, the runtime assigns values to RowPitch and DepthPitch that are greater than or equal to 8.
        *   For Texture2D, the runtime still assigns a value that isn't 0 to DepthPitch, assuming that the field isn't used.
        */
        return mappedResources;
    }

    void BaseContextWrapper::UnmapResource(ID3D11Resource* resource, unsigned int subresourceIndex)
    {
        //https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-subresources
        mD3D11Context->Unmap(resource, subresourceIndex);
    }

    void BaseContextWrapper::SetRenderTargets(unsigned int rtCount, RenderTargetView** rtViews, DepthStencilView* dsView)
    {
        ID3D11DepthStencilView* depthStencilView = dsView ? dynamic_cast<DepthStencilD3D11*>(dsView)->GetDepthStencilView() : nullptr;
        if (rtCount > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
        {
            rtCount = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        }
        if (rtCount > 0)
        {
            for (unsigned int index = 0; index < rtCount; index++)
            {
                if (rtViews[index] != nullptr)
                {
                    mRenderTargetCache.mRenderTargetViews[index] = dynamic_cast<RenderTargetD3D11*>(rtViews[index])->GetRenderTargetView();
                }
                else
                {
                    mRenderTargetCache.mRenderTargetViews[index] = nullptr;
                }
            }
            mD3D11Context->OMSetRenderTargets(rtCount, mRenderTargetCache.mRenderTargetViews, depthStencilView);
        }
        else
        {
            mD3D11Context->OMSetRenderTargets(0, nullptr, depthStencilView);
        }
    }

    void ImmediateContextD3D11::ExecuteCommandQueue(CommandQueue* commandQueue, bool bRestoreToDefaultState)
    {
        CommandQueueD3D11* D3D11CommandQueue = dynamic_cast<CommandQueueD3D11*>(commandQueue);
        D3D11CommandQueue->OnExecute(mContextWrapper.mD3D11Context.Get(), bRestoreToDefaultState);
    }

    CommandQueue* DeferredContextD3D11::FinishRecordCommandQueue(bool bRestoreToDefaultState)
    {
        ComPtr<ID3D11CommandList> commandList;
        HRESULT retQueueCommandList = mContextWrapper.mD3D11Context->FinishCommandList(!bRestoreToDefaultState, &commandList);
        if (SUCCEEDED(retQueueCommandList))
        {
            return CommandQueueD3D11::GetOrCreateNewCommandQueue(commandList.Detach());
        }
        return nullptr;
    }
}
