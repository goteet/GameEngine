#include "Scene.h"
#include "SceneNode.h"

namespace GE
{
    void GE::SceneNode::OnComponentAttach(Component* comp)
    {
        comp->_SetSceneNode_Internal(this);
    }
}
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

    bool SceneNode::AddComponent(GE::Component* component, bool autoRelease)
    {
        int index = FindComponentIndex(component);
        if (index != -1)
        {
            ComponentWrap& wrap = mComponents[index];
            if (wrap.IsRemoved)
            {
                OnComponentAttach(component);
                wrap.Component = component;
                wrap.IsAutoRelease = autoRelease;
                wrap.IsRemoved = false;
                return true;
            }
            return false;
        }
        else
        {
            ComponentWrap wrap;
            OnComponentAttach(component);
            wrap.Component = component;
            wrap.IsAutoRelease = autoRelease;
            wrap.IsRemoved = false;
            mComponents.emplace_back(wrap);
            return true;
        }
    }

    bool SceneNode::AddComponent(GE::Component* component)
    {
        return AddComponent(component, false);
    }

    bool SceneNode::AddComponent(GE::Component* component, GE::AutoReleaseComponentHint)
    {
        return AddComponent(component, true);
    }

    bool SceneNode::RemoveComponent(GE::Component* component)
    {
        ComponentWrap* wrap = RemoveComponentWithoutReleaseInternal(component);
        if (wrap)
        {
            wrap->Component->Release();
            wrap->Component = nullptr;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool SceneNode::RemoveComponentWithoutRelease(GE::Component* component)
    {
        ComponentWrap* wrap = RemoveComponentWithoutReleaseInternal(component);
        if (wrap)
        {
            wrap->Component = nullptr;
            return true;
        }
        else
        {
            return false;
        }
    }

    SceneNode::ComponentWrap* SceneNode::RemoveComponentWithoutReleaseInternal(GE::Component* component)
    {
        int index = FindComponentIndex(component);
        if (index == -1 || mComponents[index].IsRemoved)
        {
            return nullptr;
        }

        mComponents[index].IsRemoved = true;
        mComponents[index].Component->OnRemove();
        return &(mComponents[index]);
    }

    void SceneNode::PostUpdateRemoveComponents()
    {
        auto newEnd = std::remove_if(mComponents.begin(), mComponents.end(),
            [](ComponentWrap& wrap) { return wrap.IsRemoved; });
        mComponents.erase(newEnd, mComponents.end());
    }


    bool SceneNode::HasComponent(GE::Component* component) const
    {
        int index = FindComponentIndex(component);
        if (index != -1)
        {
            return mComponents[index].IsRemoved;
        }
        return false;
    }

    bool SceneNode::IsComponentAutoRelease(GE::Component* component) const
    {
        const ComponentWrap* wrap = FindComponent(component);
        return wrap != nullptr ? wrap->IsAutoRelease : false;
    }

    GE::Component* SceneNode::GetComponentByIndex(unsigned int index)
    {
        if (index >= GetComponentCount())
        {
            return nullptr;
        }
        return mComponents[index].Component;
    }

    unsigned int SceneNode::GetComponentCount() const
    {
        return (unsigned int)mComponents.size();
    }

    const math::float4x4& SceneNode::GetLocalMatrix()
    {
        return mLocalTransform;
    }

    const math::float4x4& SceneNode::GetWorldMatrix()
    {
        return GetLocalMatrix();
    }

    void SceneNode::SetLocalPosition(const math::point3d<float>& position)
    {
        mLocalTransform.set_column3(3, position);
    }

    math::point3d<float> SceneNode::GetLocalPosition() const
    {
        return math::point3d<float>(mLocalTransform.column3(3));
    }

    void SceneNode::SetWorldPosition(const math::point3d<float>& position)
    {
        SetLocalPosition(position);
    }

    math::point3d<float> SceneNode::GetWorldPosition()
    {
        return GetLocalPosition();
    }

    void SceneNode::SetRightDirection(const math::normalized_float3& rightDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 forwardDirection = mLocalTransform.column3(2);
        math::normalized_float3 upDirection = mLocalTransform.column3(1);

        bool sameDirection = fabs(math::dot(rightDirection, upDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            upDirection = math::cross(forwardDirection, rightDirection);
            forwardDirection = math::cross(rightDirection, upDirection);
        }
        else
        {
            forwardDirection = math::cross(rightDirection, upDirection);
            upDirection = math::cross(forwardDirection, rightDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, upDirection * loalScale.y);
        mLocalTransform.set_column3(2, forwardDirection * loalScale.z);
    }

    const math::normalized_float3 SceneNode::GetRightDirection()
    {
        return mLocalTransform.column3(0);
    }

    void SceneNode::SetUpDirection(const math::normalized_float3& upDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 forwardDirection = mLocalTransform.column3(2);

        bool sameDirection = fabs(math::dot(upDirection, rightDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            rightDirection = math::cross(upDirection, forwardDirection);
            forwardDirection = math::cross(rightDirection, upDirection);
        }
        else
        {
            forwardDirection = math::cross(rightDirection, upDirection);
            rightDirection = math::cross(upDirection, forwardDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, upDirection * loalScale.y);
        mLocalTransform.set_column3(2, forwardDirection * loalScale.z);
    }

    const math::normalized_float3 SceneNode::GetUpDirection()
    {
        return mLocalTransform.column3(1);
    }

    void SceneNode::SetForwardDirection(const math::normalized_float3& forwardDirection)
    {
        math::float3 loalScale = GetLocalScale();
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 upDirection = mLocalTransform.column3(1);

        bool sameDirection = fabs(math::dot(forwardDirection, rightDirection)) + math::SMALL_NUM<float> >= 1.0f;

        if (sameDirection)
        {
            rightDirection = math::cross(forwardDirection, upDirection);
            upDirection = math::cross(forwardDirection, rightDirection);
        }
        else
        {
            upDirection = math::cross(forwardDirection, rightDirection);
            rightDirection = math::cross(upDirection, forwardDirection);
        }

        mLocalTransform.set_column3(0, rightDirection * loalScale.x);
        mLocalTransform.set_column3(1, upDirection * loalScale.y);
        mLocalTransform.set_column3(2, forwardDirection * loalScale.z);
    }

    const math::normalized_float3 SceneNode::GetForwardDirection()
    {
        return mLocalTransform.column3(2);
    }

    void SceneNode::SetLocalScale(const math::float3& scale)
    {
        math::normalized_float3 rightDirection = mLocalTransform.column3(0);
        math::normalized_float3 forwardDirection = mLocalTransform.column3(1);
        math::normalized_float3 upDirection = mLocalTransform.column3(2);
        mLocalTransform.set_column3(0, rightDirection * scale.x);
        mLocalTransform.set_column3(1, forwardDirection * scale.y);
        mLocalTransform.set_column3(2, upDirection * scale.z);
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
        mVisibleMask = mask;
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
        for (ComponentWrap& compWrap : mComponents)
        {
            if (compWrap.IsAutoRelease)
            {
                compWrap.Component->Release();
            }
        }

        mComponents.clear();
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

    bool SceneNode::DestoryChildSceneNode(SceneNode* node)
    {
        uint32_t index = node->GetIndex();
        if (index >= mChildren.size() || mChildren[index] != node)
        {
            return false;
        }

        delete mChildren[index];
        for (int siblings = index + 1; siblings < mChildren.size(); siblings++)
        {
            mChildren[siblings]->mOrderedIndexInParent -= 1;
            mChildren[siblings - 1] = mChildren[siblings];
        }
        mChildren.pop_back();

        return true;
    }

    void SceneNode::RecursiveUpdate(uint32_t elapsedMilliseconds)
    {
        for (const ComponentWrap& compWrap : mComponents)
        {
            compWrap.Component->OnUpdate(elapsedMilliseconds);
        }
        PostUpdateRemoveComponents();
        for (SceneNode* child : mChildren)
        {
            child->RecursiveUpdate(elapsedMilliseconds);
        }
    }

    void SceneNode::RecursiveRender()
    {
        for (const ComponentWrap& compWrap : mComponents)
        {
            compWrap.Component->OnRender();
        }
        for (SceneNode* child : mChildren)
        {
            child->RecursiveRender();
        }
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

    int SceneNode::FindComponentIndex(GE::Component* comp) const
    {
        for (int index = 0; index < mComponents.size(); index++)
        {
            if (mComponents[index].Component == comp)
            {
                return index;
            }
        }
        return -1;
    }

    SceneNode::ComponentWrap* SceneNode::FindComponent(GE::Component* comp)
    {
        int index = FindComponentIndex(comp);
        return index == -1 ? nullptr : &(mComponents[index]);
    }

    const SceneNode::ComponentWrap* SceneNode::FindComponent(GE::Component* comp) const
    {
        return const_cast<SceneNode*>(this)->FindComponent(comp);
    }


    InvalidateRootSceneNode::InvalidateRootSceneNode(Scene* scene)
        : SceneNode(scene, nullptr, 0)
    {

    }
}
