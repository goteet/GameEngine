#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Base/ScopeHelper.h>
#include "GEInclude.h"
#include "GameEngine.h"

namespace engine
{
    GE::RenderSystem* GameEngine::GetRenderSystem()
    {
        return mRenderSystem.get();
    }

    GE::Scene* GameEngine::CreateOrGetDefaultScene()
    {
        if (mDefualtScene == nullptr)
        {
            mDefualtScene = std::make_unique_ptr<engine::Scene>();
        }
        return mDefualtScene.get();
    }

    void GameEngine::Update(unsigned int deltaMillisec)
    {
        mRenderSystem->RenderFrame();
    }

    bool GameEngine::OnResizeWindow(void* hWndinw, unsigned int width, unsigned int height)
    {
        return mRenderSystem->OnResizeWindow(hWndinw, width, height);
    }

    bool GameEngine::InitializeMe(const GE::GameEngine::CreationConfig& config)
    {
        mRenderSystem = std::make_unique<RenderSystem>(config.NativeWindow, config.IsFullScreen, config.InitialWidth, config.InitialHeight);
        return mRenderSystem->InitializeGfxDevice();
    }

}

