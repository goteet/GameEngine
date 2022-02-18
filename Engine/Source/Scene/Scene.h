#pragma once
#include "GEInclude.h"
#include "SceneNode.h"

namespace engine
{
    class Camera;
    class Scene : public GE::Scene
    {
    public:
        DefineRTTI;

        virtual GE::Camera* CreateAdditionalCameraNode() override;
        virtual GE::Camera* GetDefaultCamera() override;
        virtual GE::Camera* GetCameraByIndex(unsigned int index) override;
        virtual unsigned int GetCameraCount() const override;
        virtual GE::SceneNode* CreateSceneNode() override;
        virtual GE::SceneNode* GetSceneNodeByIndex(unsigned int index) override;
        virtual unsigned int GetSceneNodeCount() const override;

        Scene();

        void UpdateAndRender(unsigned int elapsedMilliseconds);

        Camera* GetDefaultCameraInternal();

    private:
        void InitializeDefaultNodes();
        Camera* CreateCameraNode();
        InvalidateRootSceneNode mRoot;
        std::vector<engine::Camera*> mCameras;
    };
}
