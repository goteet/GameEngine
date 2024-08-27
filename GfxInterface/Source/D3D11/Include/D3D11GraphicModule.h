#pragma once
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct GraphicModuleD3D11 : public GraphicModule
    {
        GraphicModuleD3D11();
        virtual ~GraphicModuleD3D11();
        virtual void Release() override;

        virtual bool IsHardwareSupported() override;
        virtual GraphicDevice* CreateDevice() override;

        IDXGIFactory6* GetDXGIFactory() { return mDXGIFactory.Get(); }

    private:
        ComPtr<IDXGIFactory6> mDXGIFactory;
        ComPtr<IDXGIAdapter1> mDXGIAdapter;
    };
}
