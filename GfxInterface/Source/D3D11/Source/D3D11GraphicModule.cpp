#include "D3D11GraphicDevice.h"
#include "D3D11GraphicModule.h"

namespace GFXI
{
    const bool kDebugValidation = false;

    GraphicModuleD3D11::GraphicModuleD3D11()
        : mDXGIFactory(nullptr)
        , mDXGIAdapter(nullptr)
    {
        UINT DebugFlag = kDebugValidation ? D3D11_CREATE_DEVICE_DEBUG : 0;
        ComPtr<IDXGIFactory6> DXGIFactory;
        HRESULT retCreateDXGIFactory = CreateDXGIFactory2(DebugFlag, __uuidof(IDXGIFactory6), &DXGIFactory);

        SIZE_T bestDedicatedVideoMemory = 0;
        SIZE_T bestDedicatedSystemMemory = 0;
        ComPtr<IDXGIAdapter1> bestAdapter = nullptr;
        if (SUCCEEDED(retCreateDXGIFactory))
        {
            UINT adapterIndex = 0;
            ComPtr<IDXGIAdapter1> adapter;
            while (DXGI_ERROR_NOT_FOUND != DXGIFactory->EnumAdapters1(adapterIndex, &adapter))
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                adapter->GetDesc1(&adapterDesc);

                if (adapterDesc.Flags != DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    if (bestDedicatedVideoMemory <  adapterDesc.DedicatedVideoMemory ||
                        bestDedicatedVideoMemory == adapterDesc.DedicatedVideoMemory && bestDedicatedSystemMemory < adapterDesc.DedicatedSystemMemory)
                    {
                        bestAdapter = adapter;
                        bestDedicatedVideoMemory  = adapterDesc.DedicatedVideoMemory;
                        bestDedicatedSystemMemory = adapterDesc.DedicatedSystemMemory;
                    }
                }
                adapterIndex++;
            }

            if (bestAdapter != nullptr)
            {
                mDXGIFactory = DXGIFactory;
                mDXGIAdapter = bestAdapter;
            }
        }
    }

    GraphicModuleD3D11::~GraphicModuleD3D11()
    {
        mDXGIAdapter.Reset();
        mDXGIFactory.Reset();
    }

    bool GraphicModuleD3D11::IsHardwareSupported()
    {
        return mDXGIFactory != nullptr && mDXGIAdapter != nullptr;
    }

    GraphicDevice* GraphicModuleD3D11::CreateDevice()
    {
        if (!IsHardwareSupported())
        {
            return nullptr;
        }


        const HMODULE   kNoneSoftwareModule = nullptr;
        const UINT      kDeviceCreationFlag = 0;
        const int       kNumFeatureLevel = 2;
        D3D_FEATURE_LEVEL kFeatureLevels[kNumFeatureLevel] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        ComPtr<ID3D11Device>        D3D11Device = nullptr;
        ComPtr<ID3D11DeviceContext> deviceContextImmediate = nullptr;
        ComPtr<ID3D11DeviceContext> deviceContextDeferred = nullptr;
        D3D_FEATURE_LEVEL           deviceFeatureLevel;


        HRESULT RetCreateDevice = D3D11CreateDevice(
            mDXGIAdapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,    //D3D_DRIVER_TYPE_HARDWARE for NullAdapter.
            kNoneSoftwareModule,
            kDeviceCreationFlag,
            kFeatureLevels, kNumFeatureLevel,
            D3D11_SDK_VERSION,
            &D3D11Device,
            &deviceFeatureLevel,
            &deviceContextImmediate
        );

        if (SUCCEEDED(RetCreateDevice))
        {
            //IDXGIDevice2* DXGIDevice = nullptr;
            //D3D11Device->QueryInterface<IDXGIDevice2>(&DXGIDevice);
            HRESULT RetCreateDeferredContext = D3D11Device->CreateDeferredContext(0, &deviceContextDeferred);
            if (SUCCEEDED(RetCreateDeferredContext))
            {
                return new GraphicDeviceD3D11(this, D3D11Device.Detach(), deviceContextImmediate.Detach(), deviceContextDeferred.Detach());
            }
            //ASSERT_SUCCEEDED(resultCreateDeferredContext);
        }
        return nullptr;
    }

    void GraphicModuleD3D11::Release()
    {
        delete this;
    }
}

extern "C" GfxInterfaceAPI GFXI::GraphicModule* CreateGfxModule()
{
    return new GFXI::GraphicModuleD3D11();
}
