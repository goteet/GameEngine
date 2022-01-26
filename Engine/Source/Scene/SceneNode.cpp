#include "Scene.h"
#include "SceneNode.h"

namespace engine
{
    math::float4x4 SceneNode::dummy;
    math::float4x4 InvalidateRootSceneNode::dummy;
    GE::Scene* SceneNode::GetScene()
    {
        return mScene;
    }

    InvalidateRootSceneNode::InvalidateRootSceneNode(Scene* scene)
        : SceneNode(scene, nullptr)
    {

    }
}
