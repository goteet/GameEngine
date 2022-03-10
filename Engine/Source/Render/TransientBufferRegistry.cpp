#include "TransientBufferRegistry.h"    

namespace engine
{
    TransientBufferRegistry::TransientBufferRegistry(GfxDevice* creator, GfxRenderTarget* defaultBackbufferRT, GfxDepthStencil* defaultBackbufferDS)
        : mGfxResourceCreator(creator)
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

    GfxRenderTarget* TransientBufferRegistry::AllocateRenderTarget(ERenderTargetFormat format, unsigned int width, unsigned int height, bool usedForShader)
    {
        GfxRenderTarget* result = FindSame(mRenderTargets, format, width, height, usedForShader);
        return result != nullptr ? result
            : mGfxResourceCreator->CreateRenderTarget(format, width, height, usedForShader);
    }

    GfxDepthStencil* TransientBufferRegistry::AllocateDepthStencil(EDepthStencilFormat format, unsigned int width, unsigned int height, bool usedForShader)
    {
        GfxDepthStencil* result = FindSame(mDepthStencils, format, width, height, usedForShader);
        return result != nullptr ? result
            : mGfxResourceCreator->CreateDepthStencil(format, width, height, usedForShader);
        return nullptr;
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

    void TransientBufferRegistry::RecycleRenderTarget(GfxRenderTarget* texture)
    {
        Recycle(mRenderTargets, texture);
    }
    void TransientBufferRegistry::RecycleDepthStencil(GfxDepthStencil* texture)
    {
        Recycle(mDepthStencils, texture);
    }
    void TransientBufferRegistry::ReleaseAllBuffers()
    {
        for (GfxRenderTarget* rt : mRenderTargets)
        {
            safe_delete(rt);
        }
        mRenderTargets.clear();

        for (GfxDepthStencil* ds : mDepthStencils)
        {
            safe_delete(ds);
        }
        mDepthStencils.clear();
    }
}
