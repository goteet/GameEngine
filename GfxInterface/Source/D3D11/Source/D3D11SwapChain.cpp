#include "D3D11SwapChain.h"
#include "D3D11Texture.h"

namespace GFXI
{
    SwapChainD3D11::SwapChainD3D11(IDXGISwapChain1* swapChain, RenderTargetD3D11* defaultBackBuffer)
        : mSwapChain(swapChain)
        , mDefaultBackbuffer(defaultBackBuffer)
    {
        
    }

    SwapChainD3D11::~SwapChainD3D11()
    {
        if (mDefaultBackbuffer)
        {
            mDefaultBackbuffer->Release();
        }
        mSwapChain.Reset();
    }

    void SwapChainD3D11::Release()
    {
        delete this;
    }

    RenderTargetView* SwapChainD3D11::GetRenderTargetView()
    {
        return mDefaultBackbuffer;
    }

    void SwapChainD3D11::Present()
    {
        const UINT kInterval = 0;
        const UINT kNoFlags = 0;
        mSwapChain->Present(kInterval, kNoFlags);
    }
}
