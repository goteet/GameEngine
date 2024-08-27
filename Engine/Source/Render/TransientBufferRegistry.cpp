#include "TransientBufferRegistry.h"    

namespace engine
{
    TransientBufferRegistry::TransientBufferRegistry(GFXI::GraphicDevice* creatorPtr, GFXI::RenderTargetView* defaultBackbufferRT, GFXI::DepthStencilView* defaultBackbufferDS)
        : mGfxResourceDevice(creatorPtr)
        , mDefaultBackbufferRT(defaultBackbufferRT)
        , mDefaultBackbufferDSTemp(defaultBackbufferDS)
    {

    }

    TransientBufferRegistry::~TransientBufferRegistry()
    {
        ReleaseAllBuffers();
    }

    template<typename T, class TFormat>
    T* FindSame(std::vector<T*>& container, TFormat format, unsigned int width, unsigned int height, bool forshader)
    {
        auto it = std::find_if(container.begin(), container.end(),
            [&](T* v) { return (v != nullptr) && v->IsSame(format, width, height, forshader); }
        );
        if (it != container.end())
        {
            T* rst = *it;
            *it = nullptr;
            return rst;
        }
        return nullptr;
    }

    template<typename T, class TFormat>
    T* FindCache(std::vector<T*>& container, TFormat format, unsigned int width, unsigned int height, bool forshader)
    {
        auto it = std::find_if(container.begin(), container.end(),
            [&](T* v)
            {
                return (v != nullptr)
                    && v->GetFormat() == format
                    && v->GetWidth() == width
                    && v->GetHeight() == height
                    && v->IsUsedByShader() == forshader;
            }
        );
        if (it != container.end())
        {
            T* rst = *it;
            *it = nullptr;
            return rst;
        }
        return nullptr;
    }

    GFXI::RenderTargetView* TransientBufferRegistry::AllocateRenderTarget(GFXI::RenderTargetView::EFormat format, unsigned int width, unsigned int height, bool usedByShader)
    {
        GFXI::RenderTargetView* result = FindCache(mRenderTargets, format, width, height, usedByShader);
        if (result != nullptr)
        {
            return result;
        }

        GFXI::RenderTargetView::CreateInfo rtCreateInfo;
        rtCreateInfo.Format = format;
        rtCreateInfo.Width = width;
        rtCreateInfo.Height = height;
        rtCreateInfo.UsedByShader = usedByShader;

        return mGfxResourceDevice->CreateRenderTargetView(rtCreateInfo);
    }

    GFXI::DepthStencilView* TransientBufferRegistry::AllocateDepthStencil(GFXI::DepthStencilView::EFormat format, unsigned int width, unsigned int height, bool usedByShader)
    {
        GFXI::DepthStencilView* result = FindCache(mDepthStencils, format, width, height, usedByShader);
        if (result != nullptr)
        {
            return result;
        }

        GFXI::DepthStencilView::CreateInfo dsCreateInfo;
        dsCreateInfo.Format = format;
        dsCreateInfo.Width  = width;
        dsCreateInfo.Height = height;
        dsCreateInfo.UsedByShader = usedByShader;

        return mGfxResourceDevice->CreateDepthStencilView(dsCreateInfo);
    }

    template<typename T>
    void Recycle(std::vector<T*>& container, T* texture)
    {
        auto it = std::find_if(container.begin(), container.end(),
            [&](T* v) { return v == texture || v == nullptr; }
        );
        if (container.end() == it)
        {
            container.emplace_back(texture);
        }
        else if (*it == nullptr)
        {
            *it = texture;
        }
        else
        {
            ASSERT(false);
        }
    }

    void TransientBufferRegistry::RecycleRenderTarget(GFXI::RenderTargetView* texture)
    {
        Recycle(mRenderTargets, texture);
    }
    void TransientBufferRegistry::RecycleDepthStencil(GFXI::DepthStencilView* texture)
    {
        Recycle(mDepthStencils, texture);
    }
    void TransientBufferRegistry::ReleaseAllBuffers()
    {
        for (GFXI::RenderTargetView* rt : mRenderTargets)
        {
            SafeRelease(rt);
        }
        mRenderTargets.clear();

        for (GFXI::DepthStencilView* ds : mDepthStencils)
        {
            SafeRelease(ds);
        }
        mDepthStencils.clear();
    }
}
