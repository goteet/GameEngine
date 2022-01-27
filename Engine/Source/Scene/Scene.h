#pragma once
#include "GEInclude.h"
#include "SceneNode.h"

namespace engine
{
    class Scene : public GE::Scene
    {
    public:
        DefineRTTI;

        virtual GE::Camera* CreateAdditionalCameraNode() override { return nullptr; }
        virtual GE::Camera* GetDefaultCamera()override { return nullptr; }
        virtual GE::Camera* GetCameraByIndex(unsigned int index) override;
        virtual unsigned int GetCameraCount() const override;
        virtual GE::SceneNode* CreateSceneNode() override;
        virtual GE::SceneNode* GetSceneNodeByIndex(unsigned int index) override;
        virtual unsigned int GetSceneNodeCount() const override;

        Scene();

    private:
        InvalidateRootSceneNode mRoot;
        std::vector<GE::Camera*> mCameras;
    };
}
