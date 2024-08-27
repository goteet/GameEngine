#include "GfxInterface.h"
#include "Core/GameEngine.h"

namespace engine_gfx_impl
{
    bool InitializeD3D11Buffer(ID3D11Device* GfxDevice,
        engine::GfxBuffer& OutBuffer,
        unsigned int InBufferLength,
        D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
    {
        OutBuffer.mBufferDesc.ByteWidth = InBufferLength;
        HRESULT creation = GfxDevice->CreateBuffer(
            &OutBuffer.mBufferDesc,
            SubResourceDataPtr,
            &OutBuffer.mBufferPtr);

        if (SUCCEEDED(creation))
        {
            OutBuffer.mGfxDevicePtr = GfxDevice;
            return true;
        }
        else
        {
            OutBuffer.mBufferDesc.ByteWidth = 0;
            return false;
        }
    }

    template<typename VertexBufferType>
    VertexBufferType* CreateVertexBuffer(ID3D11Device* GfxDevice,
        unsigned int VertexStride, unsigned int VertexCount,
        D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
    {
        VertexBufferType* bufferPtr = new VertexBufferType();
        if (!InitializeD3D11Buffer(GfxDevice, *bufferPtr, VertexStride * VertexCount, SubResourceDataPtr))
        {
            safe_delete(bufferPtr);
        }
        return bufferPtr;
    }

    template<typename IndexBufferType>
    IndexBufferType* CreateIndexBuffer(ID3D11Device* GfxDevice,
        unsigned int IndexStride, unsigned int IndexCount,
        D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
    {
        DXGI_FORMAT format;
        switch (IndexStride)
        {
        case 4:
            format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
            break;

        case 2:
            format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;
            break;

        default:
            return nullptr;
        }

        IndexBufferType* bufferPtr = new IndexBufferType();
        bufferPtr->Format = format;
        if (!InitializeD3D11Buffer(GfxDevice, *bufferPtr, IndexStride * IndexCount, SubResourceDataPtr))
        {
            safe_delete(bufferPtr);
        }
        return bufferPtr;
    }

    template<typename GfxConstantBufferType>
    GfxConstantBufferType* CreateConstantBuffer(ID3D11Device* GfxDevice,
        unsigned int BufferLength,
        D3D11_SUBRESOURCE_DATA* SubResourceDataPtr = nullptr)
    {
        GfxConstantBufferType* constantBuffer = new GfxConstantBufferType();
        if (!InitializeD3D11Buffer(GfxDevice, *constantBuffer, BufferLength, SubResourceDataPtr))
        {
            safe_delete(constantBuffer);
        }
        return constantBuffer;
    }

}

namespace engine
{

    GfxDevice::GfxDevice(ID3D11Device* device)
        : mGfxDevice(device)
    { }

    GfxDevice::~GfxDevice()
    {
        SafeRelease(mGfxDevice);
    }

    GE::GfxDefaultVertexBuffer* GfxDevice::CreateDefaultVertexBuffer(unsigned int vertexCount)
    {
        return CreateDefaultVertexBufferImpl(vertexCount);
    }

    GfxDefaultVertexBuffer* GfxDevice::CreateDefaultVertexBufferImpl(unsigned int vertexCount)
    {
        using namespace engine_gfx_impl;
        return CreateVertexBuffer<GfxDefaultVertexBuffer>(mGfxDevice, sizeof(VertexLayout), vertexCount);
    }

    GE::GfxDefaultIndexBuffer* GfxDevice::CreateDefaultIndexBuffer(unsigned int indexCount)
    {
        return CreateDefaultIndexBufferImpl(indexCount);
    }
    GfxDefaultIndexBuffer* GfxDevice::CreateDefaultIndexBufferImpl(unsigned int indexCount)
    {
        using namespace engine_gfx_impl;
        return CreateIndexBuffer<GfxDefaultIndexBuffer>(mGfxDevice, sizeof(int), indexCount);
    }


    GfxDynamicConstantBuffer* GfxDevice::CreateDynamicConstantBuffer(unsigned int bufferLength)
    {
        using namespace engine_gfx_impl;
        return CreateConstantBuffer<GfxDynamicConstantBuffer>(mGfxDevice, bufferLength);
    }

    GfxRenderTarget* GfxDevice::CreateRenderTarget(ERenderTargetFormat format, unsigned int width, unsigned int height, bool usedByShader)
    {
        GfxRenderTarget* pRenderTarget = new GfxRenderTarget(format, usedByShader);
        pRenderTarget->mTexture2DDesc.Width = width;
        pRenderTarget->mTexture2DDesc.Height = height;

        HRESULT rstCreate = mGfxDevice->CreateTexture2D(&pRenderTarget->mTexture2DDesc, nullptr, pRenderTarget->mTexturePtr.ReleaseAndGetAddressOf());
        if (FAILED(rstCreate))
        {
            safe_delete(pRenderTarget);
            return nullptr;
        }

        HRESULT resultCreateRTV = mGfxDevice->CreateRenderTargetView(pRenderTarget->mTexturePtr.Get(), &pRenderTarget->mRenderTargetDesc, pRenderTarget->mRenderTargetView.ReleaseAndGetAddressOf());
        ASSERT_SUCCEEDED(resultCreateRTV);

        if (usedByShader)
        {
            HRESULT resultCreateSRV = mGfxDevice->CreateShaderResourceView(pRenderTarget->mTexturePtr.Get(), &pRenderTarget->mShaderResourceDesc, pRenderTarget->mShaderResourceView.ReleaseAndGetAddressOf());
            ASSERT_SUCCEEDED(resultCreateSRV);
        }
        return pRenderTarget;
    }

    GfxDepthStencil* GfxDevice::CreateDepthStencil(EDepthStencilFormat format, unsigned int width, unsigned int height, bool usedByShader)
    {
        //std::vector<D3D11_SUBRESOURCE_DATA> Data;
        //if (InSubResourceData != nullptr)
        //{
        //    uint32_t DataSize = InDesc->ArraySize * std::max(1u, InDesc->MipLevels);
        //    Data.resize(DataSize);
        //    for (uint32_t Slice = 0; Slice < DataSize; ++Slice)
        //        Data[Slice] = InSubResourceData[Slice];
        //}

        //https://docs.microsoft.com/en-us/windows/uwp/gaming/create-depth-buffer-resource--view--and-sampler-state

        GfxDepthStencil* pDepthStencil = new GfxDepthStencil(format, usedByShader);
        pDepthStencil->mTexture2DDesc.Width = width;
        pDepthStencil->mTexture2DDesc.Height = height;

        HRESULT rstCreate = mGfxDevice->CreateTexture2D(&pDepthStencil->mTexture2DDesc, nullptr, pDepthStencil->mTexturePtr.ReleaseAndGetAddressOf());
        if (FAILED(rstCreate))
        {
            safe_delete(pDepthStencil);
            return nullptr;
        }

        HRESULT resultCreateDSV = mGfxDevice->CreateDepthStencilView(pDepthStencil->mTexturePtr.Get(), &pDepthStencil->mDepthStencilDesc, pDepthStencil->mDepthStencilView.ReleaseAndGetAddressOf());
        ASSERT_SUCCEEDED(resultCreateDSV);

        if (usedByShader)
        {
            HRESULT resultCreateSRV = mGfxDevice->CreateShaderResourceView(pDepthStencil->mTexturePtr.Get(), &pDepthStencil->mShaderResourceDesc, pDepthStencil->mShaderResourceView.ReleaseAndGetAddressOf());
            ASSERT_SUCCEEDED(resultCreateSRV);
        }
        return pDepthStencil;
    }

    bool GfxDevice::InitializeTemporaryStagingBuffer(GfxStagingBuffer& outBuffer, unsigned int length)
    {
        using namespace engine_gfx_impl;
        return InitializeD3D11Buffer(mGfxDevice, outBuffer, length);
    }

    GfxDeviceContext::GfxDeviceContext(GfxDevice* device, ID3D11DeviceContext* context)
        : mGfxDevice(device)
        , mGfxDeviceContext(context)
    { }

    GfxDeviceContext::~GfxDeviceContext()
    {
        SafeRelease(mGfxDeviceContext);
    }

    void GfxDeviceContext::SetVertexBufferImpl(GfxBaseVertexBuffer* vb, unsigned int offset)
    {
        ID3D11Buffer* VertexBuffer = vb->GetBufferPtr();
        const UINT Stride = vb->GetVertexStride();
        const UINT& Offset = offset;
        mGfxDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    }

    void GfxDeviceContext::SetIndexBufferImpl(GfxBaseIndexBuffer* ib, unsigned int offset)
    {
        mGfxDeviceContext->IASetIndexBuffer(ib->GetBufferPtr(), ib->GetFormat(), offset);
    }

    void GfxDeviceContext::SetVSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer* constantBuffer)
    {
        ID3D11Buffer* VertexShaderBuffer = constantBuffer->GetBufferPtr();
        mGfxDeviceContext->VSSetConstantBuffers(startIndex, 1, &VertexShaderBuffer);

    }

    void GfxDeviceContext::SetPSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer* constantBuffer)
    {
        ID3D11Buffer* VertexShaderBuffer = constantBuffer->GetBufferPtr();
        mGfxDeviceContext->PSSetConstantBuffers(startIndex, 1, &VertexShaderBuffer);
    }

    void GfxDeviceContext::UnmapBuffer(GfxStagingBuffer& buffer, UINT subresource)
    {
        UnmapBufferImpl(buffer, subresource);
    }

    void GfxDeviceContext::UnmapBuffer(GfxDynamicBuffer& buffer, UINT subresource)
    {
        UnmapBufferImpl(buffer, subresource);
    }

    void* GfxDeviceContext::MapBufferImpl(GfxBuffer& buffer, UINT subresource, GfxDeviceContext::EMapMethod method)
    {
        D3D11_MAP mapType = method == Default
            ? D3D11_MAP_WRITE
            : (method == Discard
                ? D3D11_MAP_WRITE_DISCARD
                : D3D11_MAP_WRITE_NO_OVERWRITE);
        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map_flag
        //do-not-wait-when-gpu-busy
        UINT mapFlags = 0;
        D3D11_MAPPED_SUBRESOURCE outMappedSubresource;

        HRESULT result = mGfxDeviceContext->Map(buffer.mBufferPtr.Get(), subresource, mapType, mapFlags, &outMappedSubresource);
        if (SUCCEEDED(result))
        {
            return outMappedSubresource.pData;
        }
        return nullptr;
    }


    void GfxDeviceContext::UnmapBufferImpl(GfxBuffer& buffer, UINT subresource)
    {
        mGfxDeviceContext->Unmap(buffer.mBufferPtr.Get(), subresource);
    }

    void GfxDeviceContext::SetRenderTargets(GfxRenderTarget** renderTargets, unsigned int rtCount, GfxDepthStencil* ds)
    {
        rtCount = math::min2(rtCount, 8);
        if (rtCount == 0)
        {
            rtCount = 1;
            mRenderTargetViews[0] = nullptr;
        }
        else
        {
            for (unsigned int i = 0; i < rtCount; i++)
            {
                mRenderTargetViews[i] = renderTargets[i]->mRenderTargetView.Get();
            }
        }
        mGfxDeviceContext->OMSetRenderTargets(rtCount, mRenderTargetViews,
            ds == nullptr ? nullptr : ds->mDepthStencilView.Get());
    }

    void GfxDeviceContext::ClearRenderTarget(GfxRenderTarget* rt, const math::float4& color)
    {
        mGfxDeviceContext->ClearRenderTargetView(rt->mRenderTargetView.Get(), color.v);
    }

    void ClearDeptnStencil(ID3D11DeviceContext* context, ID3D11DepthStencilView* view, UINT clearFlags, float depth, unsigned char stencil)
    {
        context->ClearDepthStencilView(view, clearFlags, depth, stencil);
    }

    void GfxDeviceContext::ClearDepthOnly(GfxDepthStencil* ds, float depth)
    {
        ClearDeptnStencil(mGfxDeviceContext, ds->mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
    }

    void GfxDeviceContext::ClearStencilOnly(GfxDepthStencil* ds, unsigned char stencil)
    {
        ClearDeptnStencil(mGfxDeviceContext, ds->mDepthStencilView.Get(), D3D11_CLEAR_STENCIL, 1.0f, stencil);
    }


    void GfxDeviceContext::ClearDepthStencil(GfxDepthStencil* ds, float depth, unsigned char stencil)
    {
        ClearDeptnStencil(mGfxDeviceContext, ds->mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void GfxDeviceContext::ClearDepthStencil(GfxDepthStencil* ds, bool depth, bool stencil, float dv, unsigned char sv)
    {
        UINT flags = (depth ? D3D11_CLEAR_DEPTH : 0) | (stencil ? D3D11_CLEAR_STENCIL : 0);
        ClearDeptnStencil(mGfxDeviceContext, ds->mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    GfxImmediateContext::GfxImmediateContext(GfxDevice* device, ID3D11DeviceContext* context)
        : GfxDeviceContext(device, context)
    { }

    void GfxImmediateContext::UploadEntireBufferFromStagingMemory(GE::GfxBuffer* buffer, const void* data)
    {
        GfxBuffer* bufferImpl = dynamic_cast<GfxBuffer*>(buffer);
        UploadEntireBufferFromStagingMemoryImpl(bufferImpl, data);
    }

    void GfxImmediateContext::UploadEntireBufferFromMemory(GE::GfxBuffer* buffer, const void* data)
    {
        GfxBuffer* bufferImpl = dynamic_cast<GfxBuffer*>(buffer);
        UploadEntireBufferFromMemoryImpl(bufferImpl, data);
    }

    void GfxImmediateContext::UploadEntireBufferFromStagingMemoryImpl(GfxBuffer* buffer, const void* data)
    {
        /*
        1. CreateVertexBuffer a 2nd buffer with D3D11_USAGE_STAGING;
        -fill the second buffer using ID3D11DeviceContext::Map, ID3D11DeviceContext::Unmap;
        -use ID3D11DeviceContext::CopyResource to copy from the staging buffer to the default buffer.
        */
        GfxStagingBuffer staging(false);
        if (mGfxDevice->InitializeTemporaryStagingBuffer(staging, buffer->GetBufferLength()))
        {
            void* pDataPtr = MapBuffer(staging);
            memcpy(pDataPtr, data, buffer->GetBufferLength());
            UnmapBuffer(staging);

            mGfxDeviceContext->CopyResource(buffer->mBufferPtr.Get(), staging.mBufferPtr.Get());
        }
    }

    void GfxImmediateContext::UploadEntireBufferFromMemoryImpl(GfxBuffer* defaultBuffer, const void* data)
    {
        //2. Use ID3D11DeviceContext::UpdateSubresource to copy data from memory.
        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11calcsubresource
        UINT subresource = 0;
        UINT sourceRowPitch = 0;
        UINT sourceDepthPitch = 0;
        mGfxDeviceContext->UpdateSubresource(defaultBuffer->mBufferPtr.Get(), subresource, nullptr,
            data, sourceRowPitch, sourceDepthPitch);
    }
}
