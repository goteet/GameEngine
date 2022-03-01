#pragma once
#include "PreIncludeFiles.h"
#include <memory>
#include "Render/RenderSystem.h"
#include "Scene/Scene.h"

namespace engine
{
    class GameEngine : public GE::GameEngine
    {
    public:
        DefineRTTI;

        virtual GE::RenderSystem* GetRenderSystem() override;

        virtual GE::Scene* CreateOrGetDefaultScene() override;

        virtual void Update(unsigned int deltaMillisec) override;

        virtual void OnMessage(const GE::Message& message) override { }

        virtual bool OnResizeWindow(void* hWindow, unsigned int width, unsigned int height) override;

        bool InitializeMe(const GE::GameEngine::CreationConfig&);

        Scene* GetDefaultSceneInternal() { return mDefualtScene.get(); }

    private:
        std::unique_ptr<engine::RenderSystem> mRenderSystem;
        std::unique_ptr<engine::Scene> mDefualtScene;
    };

    GameEngine* GetEngineInstance();
}
