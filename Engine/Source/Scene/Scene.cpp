#include <GfxInterface.h>
#include "Scene.h"
#include "Components.h"

namespace engine
{
    GE::Camera* Scene::CreateAdditionalCameraNode()
    {
        return CreateCameraNode();
    }

    GE::Camera* Scene::GetDefaultCamera()
    {
        return GetDefaultCameraInternal();
    }

    GE::Camera* Scene::GetCameraByIndex(unsigned int index)
    {
        if (index < GetCameraCount())
        {
            return mCameras[index];
        }
        return nullptr;
    }

    unsigned int Scene::GetCameraCount() const
    {
        return (unsigned int)mCameras.size();
    }


    GE::DirectionalLight* Scene::CreateDirectionalLightNode()
    {
        auto directionalLightNode = mRoot.CreateSceneNode();
        engine::DirectionalLight* compDirectionalLight = new engine::DirectionalLight();
        if (!directionalLightNode->AddComponent(compDirectionalLight, GE::AutoReleaseComponent))
        {
            delete compDirectionalLight;
            mRoot.DestoryChildSceneNode(directionalLightNode);
            return nullptr;
        }
        else
        {
            mDirectionalLights.push_back(compDirectionalLight);
        }

        return compDirectionalLight;
    }

    GE::DirectionalLight* Scene::GetDirectionalLight(uint32_t index)
    {
        if (index < mDirectionalLights.size())
        {
            return mDirectionalLights[index];
        }

        return nullptr;
    }


    GE::SceneNode* Scene::CreateSceneNode()
    {
        return mRoot.CreateSceneNode();
    }

    GE::SceneNode* Scene::GetSceneNodeByIndex(unsigned int index)
    {
        return mRoot.GetChildSceneNodeByIndex(index);
    }

    unsigned int Scene::GetSceneNodeCount() const
    {
        return mRoot.GetChildrenCount();
    }

    Scene::Scene()
        : mRoot(this)
    {
        InitializeDefaultNodes();
    }

    void Scene::RecursiveUpdate(unsigned int elapsedMilliseconds)
    {
        mRoot.RecursiveUpdate(elapsedMilliseconds);
    }

    void Scene::RecursiveRender(GFXI::DeferredContext& context)
    {
        mRoot.RecursiveRender(context);
    }

    Camera* Scene::GetDefaultCameraInternal()
    {
        return mCameras[0];
    }

    void Scene::InitializeDefaultNodes()
    {
        CreateCameraNode();
    }

    Camera* Scene::CreateCameraNode()
    {
        auto cameraNode = mRoot.CreateSceneNode();
        engine::Camera* compCamera = new engine::Camera((uint32_t)mCameras.size());
        if (!cameraNode->AddComponent(compCamera, GE::AutoReleaseComponent))
        {
            delete compCamera;
            mRoot.DestoryChildSceneNode(cameraNode);
            return nullptr;
        }
        else
        {
            mCameras.push_back(compCamera);
        }
        return compCamera;
    }
}
