#include "Scene.h"

namespace engine
{
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
    { }
}
