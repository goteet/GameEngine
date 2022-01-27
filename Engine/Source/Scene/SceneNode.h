#pragma once
#include <vector>
#include "GEInclude.h"

namespace engine
{
    class Scene;

    class SceneNode : public GE::SceneNode
    {
    public:
        DefineRTTI;
        virtual GE::Scene* GetScene() override;
        virtual GE::SceneNode* GetPrevSiblingNode() override;
        virtual GE::SceneNode* GetNextSiblingNode() override;
        virtual GE::SceneNode* GetParentNode() override;
        virtual bool AddComponent(GE::Component*) override { return false; }
        virtual bool AddComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool RemoveComponent(GE::Component*) override { return false; }
        virtual bool RemoveComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool HasComponent(GE::Component*) const override { return false; }
        virtual bool IsComponentAutoRelease(GE::Component*) const override { return false; }
        virtual GE::Component* GetComponentByIndex(unsigned int index) override { return nullptr; }
        virtual unsigned int GetComponentCount() const override { return 0; }
        virtual const math::float4x4& GetLocalMatrix() override;
        virtual const math::float4x4& GetWorldMatrix() override;
        virtual GE::SceneNode* SetLocalPosition(const math::point3d<float>& Position) override;
        virtual math::point3d<float> GetLocalPosition() const override;
        virtual GE::SceneNode* SetWorldPosition(const math::point3d<float>& Position) override;
        virtual math::point3d<float> GetWorldPosition() override;
        virtual GE::SceneNode* SetRightDirection(const math::normalized_float3& right)  override;
        virtual const math::normalized_float3 GetRightDirection() override;
        virtual GE::SceneNode* SetUpDirection(const math::normalized_float3& up) override;
        virtual const math::normalized_float3 GetUpDirection() override;
        virtual GE::SceneNode* SetForwardDirection(const math::normalized_float3& forward) override;
        virtual const math::normalized_float3 GetForwardDirection() override;
        virtual GE::SceneNode* SetLocalScale(const math::float3& scale) override;
        virtual const math::float3 GetLocalScale() const override;
        virtual void SetVisible(bool) override;
        virtual bool IsVisible() const override { return mIsVisible; }
        virtual void SetStatic(bool) override;
        virtual bool IsStatic() const override { return mIsStatic; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask) override;
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint) override;
        virtual GE::CameraVisibleMask GetCameraVisibleMask() const override { return mCameraVisibleMask; }
        virtual unsigned int GetIndex() const override { return mOrderedIndexInParent; }
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) override { return pos; }
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) override { return pos; }

        virtual ~SceneNode();

    public:
        SceneNode* CreateSceneNode();
        SceneNode* GetChildSceneNodeByIndex(uint32_t index);
        uint32_t GetChildrenCount() const;

    protected:
        SceneNode(Scene* scene, SceneNode* parent, uint32_t orderIndex);

        virtual bool IsRoot() const { return false; }


    private:
        SceneNode* GetNextChildNodeByIndex(uint32_t index);
        SceneNode* GetPrevChildNodeByIndex(uint32_t index);
        Scene* mScene = nullptr;
        SceneNode* mParent = nullptr;
        std::vector<SceneNode*> mChildren;
        math::float4x4 mLocalTransform;

        uint32_t mOrderedIndexInParent;
        uint32_t mIsVisible = false;
        uint32_t mIsStatic = false;
        GE::CameraVisibleMask mCameraVisibleMask = GE::DefaultCameraVisibkeMask;
    };

    class InvalidateRootSceneNode final : public SceneNode
    {
        DefineRTTI;
    private:
        virtual SceneNode* GetPrevSiblingNode() override { return nullptr; }
        virtual SceneNode* GetNextSiblingNode() override { return nullptr; }
        virtual bool AddComponent(GE::Component*) override { return false; }
        virtual bool AddComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool RemoveComponent(GE::Component*) override { return false; }
        virtual bool RemoveComponent(GE::Component*, GE::AutoReleaseComponentHint) override { return false; }
        virtual bool HasComponent(GE::Component*) const override { return false; }
        virtual bool IsComponentAutoRelease(GE::Component*) const override { return false; }
        virtual GE::Component* GetComponentByIndex(unsigned int) override { return nullptr; }
        virtual unsigned int GetComponentCount() const override { return 0; }
        virtual SceneNode* SetLocalPosition(const math::point3d<float>&) override { return this; }
        virtual SceneNode* SetWorldPosition(const math::point3d<float>&) override { return this; }
        virtual SceneNode* SetRightDirection(const math::normalized_float3&)  override { return this; }
        virtual SceneNode* SetUpDirection(const math::normalized_float3&) override { return this; }
        virtual SceneNode* SetForwardDirection(const math::normalized_float3&) override { return this; }
        virtual void SetVisible(bool) override { }
        virtual bool IsVisible() const override { return true; }
        virtual void SetStatic(bool) override { }
        virtual bool IsStatic() const override { return true; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint) override { }

    protected:
        virtual bool IsRoot() const { return true; }

    public:
        InvalidateRootSceneNode(Scene* scene);
    };
}
