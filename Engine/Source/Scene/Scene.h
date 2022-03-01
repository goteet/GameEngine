#pragma once
#include "PreIncludeFiles.h"
#include "SceneNode.h"

namespace engine
{
    class Camera;
    class DirectionalLight;
    class Scene : public GE::Scene
    {
    public:
        DefineRTTI;

        virtual GE::Camera* CreateAdditionalCameraNode() override;
        virtual GE::Camera* GetDefaultCamera() override;
        virtual GE::Camera* GetCameraByIndex(unsigned int index) override;
        virtual unsigned int GetCameraCount() const override;

        virtual GE::DirectionalLight* CreateDirectionalLightNode() override;
        virtual GE::DirectionalLight* GetDirectionalLight(uint32_t index) override;

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
        std::vector<engine::DirectionalLight*> mDirectionalLights;
    };
}
