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

    struct GfxBuffer
    {
        virtual ~GfxBuffer() = default;

        GfxBuffer()
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

    struct GfxImmutableBuffer : public GfxBuffer
    {
        GfxImmutableBuffer()
        {
            mBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            mBufferDesc.CPUAccessFlags = 0;
        }
    };

    struct GfxDynamicBuffer : public GfxBuffer
    {
        GfxDynamicBuffer()
        {
            mBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            mBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        }
    };

    struct GfxStagingBuffer : GfxBuffer
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

    struct GfxBaseVertexBuffer
    {
        virtual ID3D11Buffer* GetBufferPtr() = 0;
        virtual unsigned int GetVertexStride() { return sizeof(VertexLayout); }
    };

    struct GfxDefaultVertexBuffer : public GfxBuffer, public GfxBaseVertexBuffer, public GE::GfxDefaultVertexBuffer
    {
        DefineRTTI;

        GfxDefaultVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
        virtual void Release() override { delete this; }
    };

    struct GfxImmutableVertexBuffer : public GfxImmutableBuffer, GfxBaseVertexBuffer
    {
        GfxImmutableVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxDynamicVertexBuffer : public GfxDynamicBuffer, public GfxBaseVertexBuffer
    {
        GfxDynamicVertexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
    };

    struct GfxBaseIndexBuffer
    {
        virtual ID3D11Buffer* GetBufferPtr() = 0;
        DXGI_FORMAT GetFormat() const { return Format; }
        DXGI_FORMAT Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
    };

    struct GfxDefaultIndexBuffer : public GfxBuffer, public GfxBaseIndexBuffer, public GE::GfxDefaultIndexBuffer
    {
        DefineRTTI;

        GfxDefaultIndexBuffer()
        {
            mBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        }
        virtual ID3D11Buffer* GetBufferPtr() override { return mBufferPtr.Get(); }
        virtual void Release() override { delete this; }
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

    struct GfxTexture2D
    {
        unsigned int GetWidth() const { return mTexture2DDesc.Width; }
        unsigned int GetHeight() const { return mTexture2DDesc.Height; }
        DXGI_FORMAT GetTextureFormat() const { return mTexture2DDesc.Format; }

        D3D11_TEXTURE2D_DESC mTexture2DDesc;
        ComPtr<ID3D11Texture2D> mTexturePtr = nullptr;

        //D3D11_BIND_SHADER_RESOURCE
        //  Bind a buffer or texture to a shader stage;
        //  this flag cannot be used with the D3D11_MAP_WRITE_NO_OVERWRITE flag.
        D3D11_SHADER_RESOURCE_VIEW_DESC mShaderResourceDesc;
        ComPtr<ID3D11ShaderResourceView> mShaderResourceView = nullptr;
    };

    enum ERenderTargetFormat
    {
        UNormRGB10A2,
        UNormRGBA8
    };

    struct GfxRenderTarget : public GfxTexture2D
    {
        GfxRenderTarget(ERenderTargetFormat format, bool usedByShader)
            : mRenderTargetFormat(format)
        {
            switch (format)
            {
            default:
            case ERenderTargetFormat::UNormRGB10A2:
                //TODO:Not test yet.
                mTexture2DDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                mRenderTargetDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                mShaderResourceDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                break;
            case ERenderTargetFormat::UNormRGBA8:
                //TODO:Not test yet.
                mTexture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                mRenderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                mShaderResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                break;
            }

            //TODO: more general way.
            unsigned int miplevels = 1;
            mTexture2DDesc.MipLevels = miplevels;
            mTexture2DDesc.ArraySize = 1;
            mTexture2DDesc.SampleDesc.Count = 1;
            mTexture2DDesc.SampleDesc.Quality = 0;
            mTexture2DDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
            mTexture2DDesc.BindFlags = usedByShader ? (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_RENDER_TARGET;;
            mTexture2DDesc.CPUAccessFlags = 0;
            mTexture2DDesc.MiscFlags = 0;

            mRenderTargetDesc.Format;
            mRenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            mRenderTargetDesc.Texture2D.MipSlice = 0;

            mShaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            mShaderResourceDesc.Texture2D.MipLevels = miplevels;
            mShaderResourceDesc.Texture2D.MostDetailedMip = 0;
        }

        bool IsSame(ERenderTargetFormat format, unsigned int width, unsigned int height, bool forshader)
        {
            return format == mRenderTargetFormat
                && width == GetWidth()
                && height == GetHeight()
                && (forshader ? mShaderResourceView != nullptr : true);
        }

        ERenderTargetFormat mRenderTargetFormat;

        D3D11_RENDER_TARGET_VIEW_DESC mRenderTargetDesc;
        ComPtr<ID3D11RenderTargetView> mRenderTargetView = nullptr;
    };

    enum EDepthStencilFormat
    {
        UNormDepth24_UIntStencil8,    //TyplessR24G8,
        FloatDepth32,
    };

    struct GfxDepthStencil : public GfxTexture2D
    {
        GfxDepthStencil(EDepthStencilFormat format, bool usedByShader)
            :mDepthStencilFormat(format)
        {
            switch (format)
            {
            default:
            case EDepthStencilFormat::UNormDepth24_UIntStencil8:
                if (!usedByShader)
                {
                    mTexture2DDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
                    mDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                    mShaderResourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                }
                else
                {
                    //break;
                    //case EDepthStencilFormat::TyplessR24G8:
                    mTexture2DDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
                    mDepthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
                    mShaderResourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                }
                break;
            case EDepthStencilFormat::FloatDepth32:
                mTexture2DDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
                mDepthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
                mShaderResourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
                break;
            }

            unsigned int miplevels = 1;
            //TODO: more general way.
            mTexture2DDesc.MipLevels = miplevels;
            mTexture2DDesc.ArraySize = 1;
            mTexture2DDesc.SampleDesc.Count = 1;
            mTexture2DDesc.SampleDesc.Quality = 0;
            mTexture2DDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
            mTexture2DDesc.BindFlags = usedByShader ? (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE) : D3D11_BIND_DEPTH_STENCIL;
            mTexture2DDesc.CPUAccessFlags = 0;
            mTexture2DDesc.MiscFlags = 0;

            mDepthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            mDepthStencilDesc.Flags = 0;
            mDepthStencilDesc.Texture2D.MipSlice = 0;

            mShaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            mShaderResourceDesc.Texture2D.MipLevels = miplevels;
            mShaderResourceDesc.Texture2D.MostDetailedMip = 0;
        }

        bool IsSame(EDepthStencilFormat format, unsigned int width, unsigned int height, bool forshader)
        {
            return format == mDepthStencilFormat
                && width == GetWidth()
                && height == GetHeight()
                && (forshader ? mShaderResourceView != nullptr : true);
        }

        EDepthStencilFormat mDepthStencilFormat;
        D3D11_DEPTH_STENCIL_VIEW_DESC mDepthStencilDesc;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView = nullptr;
    };

    class GfxDevice : public GE::GfxDevice
    {
    public:
        DefineRTTI;

        GfxDevice(ID3D11Device* device);
        ~GfxDevice();
        virtual GE::GfxDefaultVertexBuffer* CreateDefaultVertexBuffer(unsigned int vertexCount) override;
        virtual GE::GfxDefaultIndexBuffer* CreateDefaultIndexBuffer(unsigned int indexCount) override;
        GfxDefaultVertexBuffer* CreateDefaultVertexBufferImpl(unsigned int vertexCount);
        GfxDefaultIndexBuffer* CreateDefaultIndexBufferImpl(unsigned int indexCount);
        GfxDynamicConstantBuffer* CreateDynamicConstantBuffer(unsigned int bufferLength);
        GfxRenderTarget* CreateRenderTarget(ERenderTargetFormat format, unsigned int width, unsigned int height, bool usedByShader);
        GfxDepthStencil* CreateDepthStencil(EDepthStencilFormat format, unsigned int width, unsigned int height, bool usedByShader);
        bool InitializeTemporaryStagingBuffer(GfxStagingBuffer& outBuffer, unsigned int length);

        //private:
        bool PromoteToShaderResource(GfxTexture2D* texture2D);

        ID3D11Device* mGfxDevice;
    };

    class GfxDeviceContext
    {
    public:
        enum EMapMethod
        {
            Default, Discard, NoOverwrite
        };

        GfxDeviceContext(GfxDevice*, ID3D11DeviceContext*);
        ~GfxDeviceContext();
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

    public://TODO:temporary
        void SetVertexBufferImpl(GfxBaseVertexBuffer*, unsigned int);
        void SetIndexBufferImpl(GfxBaseIndexBuffer*, unsigned int);
        void SetVSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer*);
        void SetPSConstantBufferImpl(unsigned int startIndex, GfxBaseConstantBuffer*);
        void SetRenderTargets(GfxRenderTarget** rt, unsigned int rtCount, GfxDepthStencil* ds);
        void ClearRenderTarget(GfxRenderTarget* rt, const math::float4& color);
        void ClearDepthOnly(GfxDepthStencil* ds, float depth);
        void ClearStencilOnly(GfxDepthStencil* ds, unsigned char stencil);
        void ClearDepthStencil(GfxDepthStencil* ds, float depth, unsigned char stencil);
        void ClearDepthStencil(GfxDepthStencil* ds, bool depth, bool stencil, float dv, unsigned char sv);

        GfxDevice* mGfxDevice;
        ID3D11DeviceContext* mGfxDeviceContext;
        ID3D11RenderTargetView* mRenderTargetViews[8];
    private:
        void* MapBufferImpl(GfxBuffer& buffer, UINT subresource, EMapMethod method);
        void UnmapBufferImpl(GfxBuffer& buffer, UINT subresource);
    };


    class GfxImmediateContext : public GfxDeviceContext, public GE::GfxDeviceImmediateContext
    {
    public:
        DefineRTTI;

        GfxImmediateContext(GfxDevice*, ID3D11DeviceContext*);
        virtual void UploadEntireBufferFromStagingMemory(GE::GfxBuffer*, const void* data) override;
        virtual void UploadEntireBufferFromMemory(GE::GfxBuffer*, const void* data) override;
        void UploadEntireBufferFromStagingMemoryImpl(GfxBuffer*, const void* data);
        void UploadEntireBufferFromMemoryImpl(GfxBuffer*, const void* data);
    };

    class GfxDeferredContext : public GfxDeviceContext, public GE::GfxDeferredContext
    {
    public:
        DefineRTTI;

        GfxDeferredContext(GfxDevice*, ID3D11DeviceContext*);
        virtual void SetVertexBuffer(GE::GfxVertexBuffer*, unsigned int offset) override;
        virtual void SetIndexBuffer(GE::GfxIndexBuffer*, unsigned int offset) override;
        virtual void DrawIndexed(unsigned int indexCount, unsigned int startLocation, int indexOffset) override;
        virtual void SetRenderingWorldMatrixForTest(const math::float4x4&) override;
    };
}
