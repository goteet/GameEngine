#pragma once
#include <memory>
#include "GEInclude.h"
#include "Render/RenderSystem.h"

namespace engine
{
    class GameEngine : public GE::GameEngine
    {
    public:
        DefineRTTI;

        virtual GE::RenderSystem* GetRenderSystem() override;

        virtual GE::Scene* CreateNewScene() { return nullptr; }

        virtual void Update(unsigned int deltaMillisec) { }

        virtual void OnMessage(const GE::Message& message) { }

        virtual bool OnResizeWindow(void* hWindow, unsigned int width, unsigned int height) override;

        bool InitializeMe(const GE::GameEngine::CreationConfig&);

    private:
        std::unique_ptr<engine::RenderSystem> mRenderSystem;
    };
}
