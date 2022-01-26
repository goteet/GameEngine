#pragma once
#include "GEInclude.h"

namespace engine
{
    class Scene : public GE::Scene
    {
    public:
        DefineRTTI;

        virtual GE::Camera* CreateAdditionalCameraNode() override { return nullptr; }
        virtual GE::Camera* GetDefaultCamera()override { return nullptr; }
        virtual GE::Camera* GetCameraByIndex(unsigned int index)override { return nullptr; }
        virtual unsigned int GetCameraCount() const override { return 0; }
        virtual GE::SceneNode* CreateSceneNode() override { return nullptr; }
        virtual GE::SceneNode* GetSceneNodeByIndex(unsigned int index) override{ return nullptr; }
        virtual unsigned int GetSceneNodeCount() const override { return 0; }
    private:

    };
}
