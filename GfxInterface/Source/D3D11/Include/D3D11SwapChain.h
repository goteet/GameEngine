#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct GraphicModuleD3D11;
    struct RenderTargetD3D11;

    struct SwapChainD3D11 : public SwapChain
    {
        SwapChainD3D11(IDXGISwapChain1*, RenderTargetD3D11*);
        virtual ~SwapChainD3D11();
        virtual void Release() override;

        virtual void* GetRawHandleForTest() { return mSwapChain.Get(); }
        virtual RenderTargetView* GetRenderTargetView() override;
        virtual void Present() override;

    private:
        RenderTargetD3D11*      mDefaultBackbuffer;
        ComPtr<IDXGISwapChain1> mSwapChain;
    };
}
