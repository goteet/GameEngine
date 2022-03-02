#include "GfxInterface.h"


namespace engine_gfx_impl
{
    bool InitializeD3D11Buffer(ID3D11Device* GfxDevice,
        engine::GfxBaseBuffer& OutBuffer,
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

    GfxDefaultVertexBuffer* GfxDevice::CreateDefaultVertexBuffer(unsigned int vertexCount)
    {
        using namespace engine_gfx_impl;
        return CreateVertexBuffer<GfxDefaultVertexBuffer>(mGfxDevice, sizeof(VertexLayout), vertexCount);
    }

    GfxDefaultIndexBuffer* GfxDevice::CreateDefaultIndexBuffer(unsigned int indexCount)
    {
        using namespace engine_gfx_impl;
        return CreateIndexBuffer<GfxDefaultIndexBuffer>(mGfxDevice, sizeof(int), indexCount);
    }

    GfxDynamicConstantBuffer* GfxDevice::CreateDynamicConstantBuffer(unsigned int bufferLength)
    {
        using namespace engine_gfx_impl;
        return CreateConstantBuffer<GfxDynamicConstantBuffer>(mGfxDevice, bufferLength);
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


    void GfxDeviceContext::SetVertexBuffer(GE::GfxVertexBuffer* vb, unsigned int offset)
    {
        SetVertexBufferImpl((GfxBaseVertexBuffer*)vb, offset);
    }

    void GfxDeviceContext::SetIndexBuffer(GE::GfxIndexBuffer* ib, unsigned int offset)
    {
        SetIndexBufferImpl((GfxBaseIndexBuffer*)ib, offset);
    }

    void GfxDeviceContext::UnmapBuffer(GfxStagingBuffer& buffer, UINT subresource)
    {
        UnmapBufferImpl(buffer, subresource);
    }

    void GfxDeviceContext::UnmapBuffer(GfxDynamicBuffer& buffer, UINT subresource)
    {
        UnmapBufferImpl(buffer, subresource);
    }

    void* GfxDeviceContext::MapBufferImpl(GfxBaseBuffer& buffer, UINT subresource, GfxDeviceContext::EMapMethod method)
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

    void GfxDeviceContext::UnmapBufferImpl(GfxBaseBuffer& buffer, UINT subresource)
    {
        mGfxDeviceContext->Unmap(buffer.mBufferPtr.Get(), subresource);
    }


    GfxImmediateContext::GfxImmediateContext(GfxDevice* device, ID3D11DeviceContext* context)
        : GfxDeviceContext(device, context)
    { }

    bool GfxImmediateContext::UploadBufferFromStagingMemory(GfxBaseBuffer* buffer, const void* data, unsigned int length)
    {
        /*
        1. CreateVertexBuffer a 2nd buffer with D3D11_USAGE_STAGING;
        -fill the second buffer using ID3D11DeviceContext::Map, ID3D11DeviceContext::Unmap;
        -use ID3D11DeviceContext::CopyResource to copy from the staging buffer to the default buffer.
        */

        int copyLength = buffer->GetBufferLength() > length ? length : buffer->GetBufferLength();
        GfxStagingBuffer staging(false);
        if (mGfxDevice->InitializeTemporaryStagingBuffer(staging, length))
        {
            void* pDataPtr = MapBuffer(staging);
            memcpy(pDataPtr, data, copyLength);
            UnmapBuffer(staging);

            mGfxDeviceContext->CopyResource(buffer->mBufferPtr.Get(), staging.mBufferPtr.Get());
            return true;
        }
        return false;
    }

    bool GfxImmediateContext::UploadEntireBufferFromMemory(GfxBaseBuffer* defaultBuffer, const void* data, unsigned int length)
    {
        if (defaultBuffer->GetBufferLength() > length)
            return false;

        //2. Use ID3D11DeviceContext::UpdateSubresource to copy data from memory.
        //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11calcsubresource
        UINT subresource = 0;
        UINT sourceRowPitch = 0;
        UINT sourceDepthPitch = 0;
        mGfxDeviceContext->UpdateSubresource(defaultBuffer->mBufferPtr.Get(), subresource, nullptr,
            data, sourceRowPitch, sourceDepthPitch);

        return true;
    }
}
