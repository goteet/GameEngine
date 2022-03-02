#pragma once
#include "PreIncludeFiles.h"

namespace engine
{
    using namespace Microsoft::WRL;


    struct VertexLayout
    {
        math::float4 Position;
        math::float3 Normal;
        math::float2 Texcoord;
    };

    struct GfxBaseBuffer
    {
        virtual ~GfxBaseBuffer() = default;

        GfxBaseBuffer()
        {
            //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage
            // Use Default/Immutable/Dynamic,
            //  For any default buffer, use UpdateSubresource with Staging Buffer to Upload data.
            mBufferDesc.Usage = D3D11_USAGE_DEFAULT;

            //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_bind_flag
            mBufferDesc.BindFlags = 0;

            //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_cpu_access_flag
            //0 if no CPU access is necessary.
            mBufferDesc.CPUAccessFlags = 0;

            //https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
            mBufferDesc.MiscFlags = 0;

            //https://docs.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-cs-resources#structured-buffer
            //Structure Buffer :
            mBufferDesc.StructureByteStride = 0;
        }

        ID3D11Device* mGfxDevicePtr = nullptr;
        D3D11_BUFFER_DESC mBufferDesc = { 0 };
        ComPtr<ID3D11Buffer> mBufferPtr = nullptr;
        unsigned int GetBufferLength() const { return mBufferDesc.ByteWidth; }
    };

    struct GfxImmutableBuffer : public GfxBaseBuffer
    {
        GfxImmutableBuffer()
        {
            mBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            mBufferDesc.CPUAccessFlags = 0;
        }
    };

    struct GfxDynamicBuffer : public GfxBaseBuffer
    {
        GfxDynamicBuffer()
        {
            mBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            mBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        }
    };

    struct GfxStagingBuffer : GfxBaseBuffer
    {
        GfxStagingBuffer(bool CPURead, bool CPUWrite = true)
        {
            mBufferDesc.Usage = D3D11_USAGE_STAGING;
            mBufferDesc.BindFlags = 0;
            mBufferDesc.MiscFlags = 0;
            UINT WriteFlag = CPUWrite ? D3D11_CPU_ACCESS_WRITE : 0;
            UINT ReadFlag = CPURead ? D3D11_CPU_ACCESS_READ : 0;
            mBufferDesc.CPUAccessFlags = ReadFlag | WriteFlag;
            mBufferDesc.StructureByteStride = 0;
        }
    };

    struct GfxBaseVertexBuffer : public GE::GfxVertexBuffer
    {
        virtual ID3D11Buffer* GetBufferPtr() = 0;
        virtual unsigned int GetVertexStride() { return sizeof(VertexLayout); }
    };

    struct GfxDefaultVertexBuffer : public GfxBaseBuffer, public GfxBaseVertexBuffer
    {
        DefineRTTI;

        GfxDefaultVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxImmutableVertexBuffer : public GfxImmutableBuffer, GfxBaseVertexBuffer
    {
        DefineRTTI;

        GfxImmutableVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxDynamicVertexBuffer : public GfxDynamicBuffer, public GfxBaseVertexBuffer
    {
        DefineRTTI;

        GfxDynamicVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxBaseIndexBuffer : public GE::GfxIndexBuffer
    {
        virtual ID3D11Buffer* GetBufferPtr() = 0;
        DXGI_FORMAT GetFormat() const { return Format; }
        DXGI_FORMAT Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
    };

    struct GfxDefaultIndexBuffer : public GfxBaseBuffer, public GfxBaseIndexBuffer
    {
        DefineRTTI;

        GfxDefaultIndexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxBaseConstantBuffer
    {
        virtual ID3D11Buffer* GetBufferPtr() = 0;
    };

    struct GfxDynamicConstantBuffer : public GfxDynamicBuffer, public GfxBaseConstantBuffer
    {
        GfxDynamicConstantBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    class GfxDevice
    {
    public:
        GfxDevice(ID3D11Device* device);
        ~GfxDevice();
        GfxDefaultVertexBuffer* CreateDefaultVertexBuffer(unsigned int vertexCount);
        GfxDefaultIndexBuffer* CreateDefaultIndexBuffer(unsigned int indexCount);
        GfxDynamicConstantBuffer* CreateDynamicConstantBuffer(unsigned int bufferLength);
        bool InitializeTemporaryStagingBuffer(GfxStagingBuffer& outBuffer, unsigned int length);

    //private:
        ID3D11Device* mGfxDevice;
    };

    class GfxDeviceContext : public GE::GfxDeviceContext
    {
    public:
        DefineRTTI;

        enum EMapMethod
        {
            Default, Discard, NoOverwrite
        };

        GfxDeviceContext(GfxDevice*, ID3D11DeviceContext*);
        ~GfxDeviceContext();
        virtual void SetVertexBuffer(GE::GfxVertexBuffer*, unsigned int offset) override;
        virtual void SetIndexBuffer(GE::GfxIndexBuffer*, unsigned int offset) override;
        template<typename ReturnType = void>
        ReturnType* MapBuffer(GfxStagingBuffer& buffer, UINT subresource = 0)
        {
            return (ReturnType*)MapBufferImpl(buffer, subresource, EMapMethod::Default);
        }
        template<typename ReturnType = void>
        ReturnType* MapBuffer(GfxDynamicBuffer& buffer, UINT subresource = 0)
        {
            return (ReturnType*)MapBufferImpl(buffer, subresource, EMapMethod::Discard);
        }
        void UnmapBuffer(GfxStagingBuffer& buffer, UINT subresource = 0);
        void UnmapBuffer(GfxDynamicBuffer& buffer, UINT subresource = 0);

    //TODO:temp
    //protected:
        void SetVertexBufferImpl(GfxBaseVertexBuffer*, unsigned int);
        void SetIndexBufferImpl(GfxBaseIndexBuffer*, unsigned int);
        void SetVSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer*);
        void SetPSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer*);


        GfxDevice* mGfxDevice;
        ID3D11DeviceContext* mGfxDeviceContext;

    private:
        void* MapBufferImpl(GfxBaseBuffer& buffer, UINT subresource, EMapMethod method);
        void UnmapBufferImpl(GfxBaseBuffer& buffer, UINT subresource);
    };

    class GfxImmediateContext : public GfxDeviceContext
    {
        DefineRTTI;

        GfxImmediateContext(GfxDevice*, ID3D11DeviceContext*);
        bool UploadBufferFromStagingMemory(GfxBaseBuffer*, const void* data, unsigned int length);
        bool UploadEntireBufferFromMemory(GfxBaseBuffer*, const void* data, unsigned int length);
    };
}
