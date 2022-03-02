#pragma once
#include "PreIncludeFiles.h"
#include <vector>
#include "..\..\Include\GEScene.h"

namespace engine
{
    class Scene;
    class GfxDeferredContext;

    class SceneNode : public GE::SceneNode
    {
    public:
        DefineRTTI;
        virtual GE::Scene* GetScene() override;
        virtual GE::SceneNode* GetPrevSiblingNode() override;
        virtual GE::SceneNode* GetNextSiblingNode() override;
        virtual GE::SceneNode* GetParentNode() override;
        virtual bool AddComponent(GE::Component*) override;
        virtual bool AddComponent(GE::Component*, GE::AutoReleaseComponentHint) override;
        virtual bool RemoveComponent(GE::Component*) override;
        virtual bool RemoveComponentWithoutRelease(GE::Component*) override;
        virtual bool HasComponent(GE::Component*) const override;
        virtual bool IsComponentAutoRelease(GE::Component*) const override;
        virtual GE::Component* GetComponentByIndex(unsigned int index) override;
        virtual unsigned int GetComponentCount() const override;
        virtual const math::float4x4& GetLocalMatrix() override;
        virtual const math::float4x4& GetWorldMatrix() override;
        virtual void SetLocalPosition(const math::point3d<float>& Position) override;
        virtual math::point3d<float> GetLocalPosition() const override;
        virtual void SetWorldPosition(const math::point3d<float>& Position) override;
        virtual math::point3d<float> GetWorldPosition() override;
        virtual void SetRightDirection(const math::normalized_float3& right)  override;
        virtual const math::normalized_float3 GetRightDirection() override;
        virtual void SetUpDirection(const math::normalized_float3& up) override;
        virtual const math::normalized_float3 GetUpDirection() override;
        virtual void SetForwardDirection(const math::normalized_float3& forward) override;
        virtual const math::normalized_float3 GetForwardDirection() override;
        virtual void SetLocalScale(const math::float3& scale) override;
        virtual const math::float3 GetLocalScale() const override;
        virtual void SetVisible(bool) override;
        virtual bool IsVisible() const override { return mIsVisible; }
        virtual void SetStatic(bool) override;
        virtual bool IsStatic() const override { return mIsStatic; }
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask) override;
        virtual void SetCameraVisibleMask(GE::CameraVisibleMask mask, GE::RecursiveSetCameraVisibleMaskHint) override;
        virtual GE::CameraVisibleMask GetCameraVisibleMask() const override { return mVisibleMask; }
        virtual unsigned int GetIndex() const override { return mOrderedIndexInParent; }
        virtual math::point3d<float> WorldToLocal(const math::point3d<float>& pos) override { return pos; }
        virtual math::point3d<float> LocalToWorld(const math::point3d<float>& pos) override { return pos; }

        virtual ~SceneNode();

    public:
        SceneNode* CreateSceneNode();
        SceneNode* GetChildSceneNodeByIndex(uint32_t index);
        uint32_t GetChildrenCount() const;
        bool DestoryChildSceneNode(SceneNode* node);

        void RecursiveUpdate(uint32_t elapsedMilliseconds);
        void RecursiveRender(GfxDeferredContext*);
        

    protected:
        SceneNode(Scene* scene, SceneNode* parent, uint32_t orderIndex);
        virtual bool IsRoot() const { return false; }


    private:
        struct ComponentWrap
        {
            GE::Component* Component = nullptr;
            bool IsAutoRelease = false;
            bool IsRemoved = false;
        };
        SceneNode* GetNextChildNodeByIndex(uint32_t index);
        SceneNode* GetPrevChildNodeByIndex(uint32_t index);
        int FindComponentIndex(GE::Component* comp) const;
        ComponentWrap* FindComponent(GE::Component* comp);
        const ComponentWrap* FindComponent(GE::Component* comp) const;
        bool AddComponent(GE::Component*, bool autorelease);
        ComponentWrap* RemoveComponentWithoutReleaseInternal(GE::Component*);
        void PostUpdateRemoveComponents();
        Scene* mScene = nullptr;
        SceneNode* mParent = nullptr;
        std::vector<SceneNode*> mChildren;
        std::vector<ComponentWrap> mComponents;
        math::float4x4 mLocalTransform;

        uint32_t mOrderedIndexInParent;
        uint32_t mIsVisible = false;
        uint32_t mIsStatic = false;
        GE::CameraVisibleMask mVisibleMask = GE::DefaultCameraVisibkeMask;
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
        virtual bool RemoveComponentWithoutRelease(GE::Component*) override { return false; }
        virtual bool HasComponent(GE::Component*) const override { return false; }
        virtual bool IsComponentAutoRelease(GE::Component*) const override { return false; }
        virtual GE::Component* GetComponentByIndex(unsigned int) override { return nullptr; }
        virtual unsigned int GetComponentCount() const override { return 0; }
        virtual void SetLocalPosition(const math::point3d<float>&) override { }
        virtual void SetWorldPosition(const math::point3d<float>&) override { }
        virtual void SetRightDirection(const math::normalized_float3&)  override { }
        virtual void SetUpDirection(const math::normalized_float3&) override { }
        virtual void SetForwardDirection(const math::normalized_float3&) override { }
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
