#include "RenderSystem.h"
#include <Foundation/Base/ScopeHelper.h>
#include <Foundation/Base/MemoryHelper.h>

Microsoft::WRL::ComPtr<IDXGIFactory2> GetDXGIAdapterFromDevice(Microsoft::WRL::ComPtr<ID3D11Device> pDevice)
{
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIDevice2> DXGIDevice2;
    HRESULT resultGetDXGIDevice2 = pDevice.As(&DXGIDevice2);
    if (SUCCEEDED(resultGetDXGIDevice2))
    {
        ComPtr<IDXGIAdapter1> DXGIDevice1;
        HRESULT resultGetDXGIDevice1 = DXGIDevice2->GetParent(IID_PPV_ARGS(&DXGIDevice1));
        if (SUCCEEDED(resultGetDXGIDevice1))
        {
            ComPtr<IDXGIFactory2> mDXGIFactory;
            if (SUCCEEDED(DXGIDevice1->GetParent(IID_PPV_ARGS(&mDXGIFactory))))
            {
                return mDXGIFactory;
            }
        }
    }
    return nullptr;
}

namespace engine
{
    RenderSystem::RenderSystem(void* hWindow, bool fullscreen, int width, int height)
        : mMainWindowHandle((HWND)hWindow)
        , mIsFullScreen(fullscreen)
        , mWindowWidth(width)
        , mWindowHeight(height)
    {
        RECT clientRect;
        ::GetClientRect(mMainWindowHandle, &clientRect);
        mClientWidth = clientRect.right - clientRect.left;
        mClientHeight = clientRect.bottom - clientRect.top;
    }

    bool RenderSystem::InitializeGfxDevice()
    {
        //Create Device and DeviceContext
        ComPtr<ID3D11Device> outD3DDevice = nullptr;
        ComPtr<ID3D11DeviceContext> outD3DDeviceImmediateContext = nullptr;
        ComPtr<IDXGISwapChain1> outSwapChain = nullptr;
        ComPtr<ID3D11Texture2D> outBackbuffer = nullptr;
        ComPtr<ID3D11RenderTargetView> outBackbufferRTV = nullptr;
        D3D_FEATURE_LEVEL outFeatureLevel;

        IDXGIAdapter* defualtAdpater = nullptr;
        HMODULE noneSoftwareModule = nullptr;
        UINT deviceCreationFlag = 0;
        const int numFeatureLevel = 2;
        D3D_FEATURE_LEVEL featureLevels[numFeatureLevel] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        HRESULT resultCreateDevice = D3D11CreateDevice(
            defualtAdpater,
            D3D_DRIVER_TYPE_HARDWARE,
            noneSoftwareModule,
            deviceCreationFlag,
            featureLevels, numFeatureLevel,
            D3D11_SDK_VERSION,
            &outD3DDevice,
            &outFeatureLevel,
            &outD3DDeviceImmediateContext
        );

        if (FAILED(resultCreateDevice))
        {
            return false;
        }

        ///* Create SwapChain */

        DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
        swapchainDesc.Width = mClientWidth;
        swapchainDesc.Height = mClientHeight;
        swapchainDesc.BufferCount = 2;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.Stereo = false;
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.SampleDesc.Quality = 0;
        swapchainDesc.Flags = 0;
        swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
        swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
        
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
        fullScreenDesc.RefreshRate.Numerator = 60;
        fullScreenDesc.RefreshRate.Denominator = 1;
        fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        fullScreenDesc.Windowed = !mIsFullScreen;

        ComPtr<IDXGIFactory2> DXGIFactory = GetDXGIAdapterFromDevice(outD3DDevice);
        if (DXGIFactory == nullptr)
        {
            return false;
        }

        IDXGIOutput* noneDXGIOutput = nullptr;
        HRESULT resultCreateSwapchain = DXGIFactory->CreateSwapChainForHwnd(outD3DDevice.Get(), mMainWindowHandle, &swapchainDesc, &fullScreenDesc, noneDXGIOutput, &outSwapChain);

        if (FAILED(resultCreateSwapchain))
        {
            return false;
        }

        //Create RenderTargetView
        HRESULT resultGetBackbuffer = outSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &outBackbuffer);
        if (FAILED(resultGetBackbuffer))
        {
            return false;
        }


        HRESULT resultCreateBackbufferRT = outD3DDevice->CreateRenderTargetView(outBackbuffer.Get(), nullptr, &outBackbufferRTV);
        if (FAILED(resultCreateBackbufferRT))
        {
            return false;
        }

        mGfxDevice = outD3DDevice;
        mGfxDeviceContext = outD3DDeviceImmediateContext;
        mGfxSwapChain = outSwapChain;
        mBackbuffer = outBackbuffer;
        mBackbufferRTV = outBackbufferRTV;

        return true;
    }

    void RenderSystem::RenderFrame()
    {
        auto RenderTagetView = mBackbufferRTV.Get();
        float ClearColor[4] = { 0.9f, 0.7f, 0.5f, 1.0f };
        mGfxDeviceContext->OMSetRenderTargets(1, &RenderTagetView, nullptr);

        mGfxDeviceContext->ClearRenderTargetView(RenderTagetView, ClearColor);

        D3D11_VIEWPORT Viewport;
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width = (float)mClientWidth;
        Viewport.Height = (float)mClientHeight;
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;

        mGfxDeviceContext->RSSetViewports(1, &Viewport);

        mGfxSwapChain->Present(0, 0);
    }

    bool RenderSystem::OnResizeWindow(void* hWindow, int width, int height)
    {
        if (mMainWindowHandle == (HWND)hWindow)
        {
            mWindowWidth = width;
            mWindowHeight = height;

            RECT clientRect;
            ::GetClientRect(mMainWindowHandle, &clientRect);
            mClientWidth = clientRect.right - clientRect.left;
            mClientHeight = clientRect.bottom - clientRect.top;
            return true;
        }
        return false;
    }
}
