#include "Scene.h"
#include "SceneNode.h"

namespace engine
{
    GE::Scene* SceneNode::GetScene()
    {
        return mScene;
    }

    GE::SceneNode* SceneNode::GetPrevSiblingNode()
    {
        return mParent->GetNextChildNodeByIndex(mOrderedIndexInParent);
    }

    GE::SceneNode* SceneNode::GetNextSiblingNode()
    {
        return mParent->GetPrevChildNodeByIndex(mOrderedIndexInParent);
    }

    GE::SceneNode* SceneNode::GetParentNode()
    {
        if (mParent != nullptr && mParent->IsRoot())
        {
            return nullptr;
        }
        return mParent;
    }

    const math::float4x4& SceneNode::GetLocalMatrix()
    {
        return mLocalTransform;
    }

    const math::float4x4& SceneNode::GetWorldMatrix()
    {
        return GetLocalMatrix();
    }

    GE::SceneNode* SceneNode::SetLocalPosition(const math::point3d<float>& position)
    {
        mLocalTransform.set_column3(3, position);
        return this;
    }

    math::point3d<float> SceneNode::GetLocalPosition() const
    {
        return math::point3d<float>(mLocalTransform.column3(3));
    }

    GE::SceneNode* SceneNode::SetWorldPosition(const math::point3d<float>& position)
    {
        SetLocalPosition(position);
        return this;
    }

    math::point3d<float> SceneNode::GetWorldPosition()
    {
        return GetLocalPosition();
    }

    GE::SceneNode* SceneNode::SetRightDirection(const math::normalized_float3& rightDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 forwardDirection = mLocalTransform.column3(1);
        math::normalized_float3 upDirection = mLocalTransform.column3(2);

        bool sameDirection = fabs(math::dot(rightDirection, upDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            upDirection = math::cross(rightDirection, forwardDirection);
            forwardDirection = math::cross(upDirection, rightDirection);
        }
        else
        {
            forwardDirection = math::cross(upDirection, rightDirection);
            upDirection = math::cross(rightDirection, forwardDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, forwardDirection * loalScale.y);
        mLocalTransform.set_column3(2, upDirection * loalScale.z);
        return this;
    }

    const math::normalized_float3 SceneNode::GetRightDirection()
    {
        return mLocalTransform.column3(0);
    }

    GE::SceneNode* SceneNode::SetUpDirection(const math::normalized_float3& upDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 forwardDirection = mLocalTransform.column3(1);

        bool sameDirection = fabs(math::dot(upDirection, rightDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            rightDirection = math::cross(forwardDirection, upDirection);
            forwardDirection = math::cross(upDirection, rightDirection);
        }
        else
        {
            forwardDirection = math::cross(upDirection, rightDirection);
            rightDirection = math::cross(forwardDirection, upDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, forwardDirection * loalScale.y);
        mLocalTransform.set_column3(2, upDirection * loalScale.z);
        return this;
    }

    const math::normalized_float3 SceneNode::GetUpDirection()
    {
        return mLocalTransform.column3(2);
    }

    GE::SceneNode* SceneNode::SetForwardDirection(const math::normalized_float3& forwardDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 upDirection = mLocalTransform.column3(2);

        bool sameDirection = fabs(math::dot(forwardDirection, rightDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            rightDirection = math::cross(forwardDirection, upDirection);
            upDirection = math::cross(forwardDirection, rightDirection);
        }
        else
        {
            upDirection = math::cross(forwardDirection, rightDirection);
            rightDirection = math::cross(forwardDirection, upDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, forwardDirection * loalScale.y);
        mLocalTransform.set_column3(2, upDirection * loalScale.z);
        return this;
    }

    const math::normalized_float3 SceneNode::GetForwardDirection()
    {
        return mLocalTransform.column3(2);
    }

    GE::SceneNode* SceneNode::SetLocalScale(const math::float3& scale)
    {
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 forwardDirection = mLocalTransform.column3(1);
        math::normalized_float3 upDirection = mLocalTransform.column3(2);
        mLocalTransform.set_column3(0, rightDirection * scale.x);
        mLocalTransform.set_column3(1, forwardDirection * scale.y);
        mLocalTransform.set_column3(2, upDirection * scale.z);
        return this;
    }

    const math::float3 SceneNode::GetLocalScale() const
    {
        float x = magnitude(mLocalTransform.column3(0));
        float y = magnitude(mLocalTransform.column3(1));
        float z = magnitude(mLocalTransform.column3(2));
        return math::float3(x, y, z);
    }

    void SceneNode::SetVisible(bool enable)
    {
        mIsVisible = enable;
    }

    void SceneNode::SetStatic(bool enable)
    {
        mIsStatic = enable;
    }

    void SceneNode::SetCameraVisibleMask(GE::CameraVisibleMask mask)
    {
        mCameraVisibleMask = mask;
    }

    void SceneNode::SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint hint)
    {
        SetCameraVisibleMask(mask);
        for (SceneNode* child : mChildren)
        {
            child->SetCameraVisibleMask(mask, hint);
        }
    }

    SceneNode::~SceneNode()
    {
        for (SceneNode* child : mChildren)
        {
            delete child;
        }
        mChildren.clear();
    }

    SceneNode* SceneNode::CreateSceneNode()
    {
        SceneNode* newChildNode = new SceneNode(mScene, this, (uint32_t)mChildren.size());
        mChildren.push_back(newChildNode);
        return newChildNode;
    }

    SceneNode* SceneNode::GetChildSceneNodeByIndex(uint32_t index)
    {
        if (index < GetChildrenCount())
        {
            return mChildren[index];
        }
        return nullptr;
    }

    uint32_t SceneNode::GetChildrenCount() const
    {
        return (uint32_t)mChildren.size();
    }

    SceneNode::SceneNode(Scene* scene, SceneNode* parent, uint32_t orderIndex)
        : mScene(scene)
        , mParent(parent)
        , mOrderedIndexInParent(orderIndex)
    {

    }

    SceneNode* SceneNode::GetNextChildNodeByIndex(uint32_t index)
    {
        uint32_t nextIndex = index + 1;
        return nextIndex < mChildren.size() ? mChildren[nextIndex] : nullptr;
    }

    SceneNode* SceneNode::GetPrevChildNodeByIndex(uint32_t index)
    {
        return index > 0 && index <= mChildren.size() ? mChildren[index - 1] : nullptr;
    }

    InvalidateRootSceneNode::InvalidateRootSceneNode(Scene* scene)
        : SceneNode(scene, nullptr, 0)
    {

    }
}
