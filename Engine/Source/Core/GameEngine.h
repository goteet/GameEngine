#include "GEInclude.h"

namespace engine
{
    class GameEngine : public GE::GameEngine
    {
    public:
        DefineRTTI

        virtual GE::RenderSystem* GetRenderSystem() { return nullptr; }

        virtual GE::Scene* CreateNewScene() { return nullptr; }

        virtual void Update(unsigned int deltaMillisec) { }

        virtual void OnMessage(const GE::Message& message) { }

        virtual bool OnResize(unsigned int width, unsigned int height) { return true; }

        bool InitializeMe(const GE::GameEngine::CreationConfig& ) { return true; }
    };
}
