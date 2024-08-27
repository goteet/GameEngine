#include "D3D11Texture.h"


namespace GFXI
{
    ShaderResourceViewD3D11::ShaderResourceViewD3D11(ID3D11ShaderResourceView* srView)
        : mD3D11ShaderResourceView(srView)
    {
    }
    ShaderResourceViewD3D11::~ShaderResourceViewD3D11()
    {
        mD3D11ShaderResourceView.Reset();
    }
    void ShaderResourceViewD3D11::Release()
    {
        delete this;
    }

    RenderTargetD3D11::RenderTargetD3D11(EFormat format, unsigned int width, unsigned int height, bool usedByShader
        , ID3D11Texture2D* rtTexture, ID3D11RenderTargetView* rtView, ID3D11ShaderResourceView* srView)
        : mFormat(format)
        , mWidth(width)
        , mHeight(height)
        , mUsedByShader(usedByShader)
        , mD3D11Texture2D(rtTexture)
        , mRenderTargetView(rtView)
        , mShaderResourceView(srView)
    {

    }

    RenderTargetD3D11::~RenderTargetD3D11()
    {
        mShaderResourceView.mD3D11ShaderResourceView.Reset();
        mRenderTargetView.Reset();
        mD3D11Texture2D.Reset();
    }

    void RenderTargetD3D11::Release()
    {
        delete this;
    }

    DepthStencilD3D11::DepthStencilD3D11(EFormat format, unsigned int width, unsigned int height, bool usedByShader
        , ID3D11Texture2D* dsTexture, ID3D11DepthStencilView* dsView, ID3D11ShaderResourceView* srView)
        : mFormat(format)
        , mWidth(width)
        , mHeight(height)
        , mUsedByShader(usedByShader)
        , mD3D11Texture2D(dsTexture)
        , mDepthStencilView(dsView)
        , mShaderResourceView(srView)
    {

    }

    DepthStencilD3D11::~DepthStencilD3D11()
    {
        mShaderResourceView.mD3D11ShaderResourceView.Reset();
        mDepthStencilView.Reset();
        mD3D11Texture2D.Reset();
    }

    void DepthStencilD3D11::Release()
    {
        delete this;
    }
}
