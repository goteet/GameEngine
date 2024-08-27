#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct ShaderResourceViewD3D11 : public ShaderResourceView
    {
        ShaderResourceViewD3D11(ID3D11ShaderResourceView*);
        ~ShaderResourceViewD3D11();
        virtual void Release() override;
        ID3D11ShaderResourceView* GetD3D11ShaderResourceView() { return mD3D11ShaderResourceView.Get(); }
        ComPtr<ID3D11ShaderResourceView> mD3D11ShaderResourceView = nullptr;
    };

    struct RenderTargetD3D11 : public RenderTargetView
    {
        RenderTargetD3D11(EFormat format, unsigned int width, unsigned int height, bool usedByShader
            , ID3D11Texture2D*, ID3D11RenderTargetView*, ID3D11ShaderResourceView*);
        ~RenderTargetD3D11();
        virtual void Release() override;

        virtual unsigned int    GetWidth()  override { return mWidth; }
        virtual unsigned int    GetHeight() override { return mHeight; }
        virtual EFormat         GetFormat() override { return mFormat; }
        virtual EDataUsage      GetUsage()  override { return EDataUsage::Default; }
        virtual bool IsUsedByShader() override { return mUsedByShader; }
        virtual ShaderResourceView* GetShaderResourceView() override { return &mShaderResourceView; }

        ID3D11RenderTargetView*     GetRenderTargetView()   { return mRenderTargetView.Get(); }
        ID3D11ShaderResourceView*   GetShaderResourceViewImpl() { return mShaderResourceView.GetD3D11ShaderResourceView(); }
    private:
        EFormat mFormat;
        unsigned int mWidth;
        unsigned int mHeight;
        bool mUsedByShader;
        ComPtr<ID3D11Texture2D>         mD3D11Texture2D = nullptr;
        ComPtr<ID3D11RenderTargetView>  mRenderTargetView = nullptr;
        ShaderResourceViewD3D11         mShaderResourceView;
    };

    struct DepthStencilD3D11 : public DepthStencilView
    {
        DepthStencilD3D11(EFormat format, unsigned int width, unsigned int height, bool usedByShader
            , ID3D11Texture2D*, ID3D11DepthStencilView*, ID3D11ShaderResourceView*);
        ~DepthStencilD3D11();
        virtual void Release() override;

        virtual unsigned int    GetWidth()  override { return mWidth; }
        virtual unsigned int    GetHeight() override { return mHeight; }
        virtual EFormat         GetFormat() override { return mFormat; }
        virtual EDataUsage      GetUsage()  override { return EDataUsage::Default; }
        virtual bool IsUsedByShader() override { return mUsedByShader; }
        virtual ShaderResourceView* GetShaderResourceView() override { return &mShaderResourceView; }

        ID3D11DepthStencilView*     GetDepthStencilView()   { return mDepthStencilView.Get(); }
        ID3D11ShaderResourceView* GetShaderResourceViewImpl() { return mShaderResourceView.GetD3D11ShaderResourceView(); }
    private:
        EFormat mFormat;
        unsigned int mWidth;
        unsigned int mHeight;
        bool mUsedByShader;
        ComPtr<ID3D11Texture2D>         mD3D11Texture2D = nullptr;
        ComPtr<ID3D11DepthStencilView>  mDepthStencilView = nullptr;
        ShaderResourceViewD3D11         mShaderResourceView = nullptr;
    };
}



