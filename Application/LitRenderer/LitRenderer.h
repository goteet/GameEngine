#pragma once
#include <memory>
#include <vector>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>

using F = double;

class RenderCanvas
{
public:
    RenderCanvas(unsigned char* canvasDataPtr, int width, int height, int linePitch);
    ~RenderCanvas();

    bool NeedUpdate();

    math::vector3<F>* GetBackbufferPtr() { return mBackbuffer; }
    bool NeedFlushBackbuffer = true;
    const int CanvasWidth;
    const int CanvasHeight;
    const int CanvasLinePitch;
    void IncreaseSampleCount() { mSampleCount++; }
    int GetmSampleCount() const { return mSampleCount; }

private:
    void FlushLinearColorToGammaCorrectedCanvasData();

    bool mNeedUpdateWindowRect = false;
    unsigned char* mOutCanvasDataPtr;
    math::vector3<F>* mBackbuffer = nullptr;
    int mSampleCount = 0;
};

struct SceneObject;

struct HitRecord
{
    HitRecord() = default;
    HitRecord(SceneObject* o, bool f, math::vector3<F> n, F d)
        : Object(o), SurfaceNormal(n), Distance(d), IsFrontFace(f)
    {

    }

    SceneObject* Object = nullptr;
    bool IsFrontFace = true;
    math::vector3<F> SurfaceNormal;
    F Distance = F(0);
};

struct IMaterial
{
    virtual ~IMaterial() { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering) const = 0;
    virtual math::vector3<F> Emitting() const { return math::vector3<F>::zero(); }
    virtual math::vector3<F> BRDF() const { return math::vector3<F>::zero(); }
    virtual F ScatteringPDF(const math::vector3<F>& N, const math::vector3<F>& Scattering) const { return F(1) / math::PI<F>; }
};

struct Lambertian : public IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();

    Lambertian() = default;
    Lambertian(F r, F g, F b) : Albedo(r, g, b) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering) const override;
    virtual math::vector3<F> BRDF() const override { return Albedo / math::PI<F>; }
    virtual F ScatteringPDF(const math::vector3<F>& N, const math::vector3<F>& Scattering) const override;
};

struct Metal : public IMaterial
{
    F Fuzzy = F(0);

    Metal() = default;
    Metal(F f) : Fuzzy(f) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering) const override;
    virtual math::vector3<F> BRDF() const override { return math::vector3<F>::one(); }
};

struct Dielectric : public IMaterial
{
    F RefractiveIndex = F(1.0003);

    Dielectric() = default;
    Dielectric(F ior) : RefractiveIndex(ior) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering) const override;
    virtual math::vector3<F> BRDF() const override { return math::vector3<F>::one(); }
};

struct PureLight_ForTest : IMaterial
{
    math::vector3<F> Emission = math::vector3<F>::one();

    PureLight_ForTest() = default;
    PureLight_ForTest(F x, F y, F z) : Emission(x, y, z) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering) const override;
    virtual math::vector3<F> Emitting() const override;
};

struct Transform
{
    void UpdateWorldTransform();
    math::vector3<F> Translate = math::vector3<F>::zero();
    math::quaternion<F> Rotation = math::quaternion<F>::identity();
    math::matrix4x4<F> TransformMatrix = math::matrix4x4<F>::identity();
};

struct SceneObject
{
    virtual ~SceneObject() { }
    virtual void UpdateWorldTransform();
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray, F error) const = 0;
    void SetTranslate(F x, F y, F z) { Transform.Translate.set(x, y, z); }
    void SetRotation(const math::quaternion<F>& q) { Transform.Rotation = q; }
    virtual math::point3d<F> SampleRandomPoint(math::vector3<F>& outN, F& outPDF) { return math::vector3<F>::zero(); }
    virtual bool IsDualface() const { return false; }
    Transform Transform;
    std::unique_ptr<IMaterial> Material;
};


struct SceneSphere : SceneObject
{
    SceneSphere() : mSphere(math::point3d<F>(), 1) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetRadius(F radius) { mSphere.set_radius(radius); }
private:
    math::sphere<F> mSphere;
    math::point3d<F> mWorldCenter;
};

struct SceneRect : SceneObject
{
    SceneRect() : Rect(math::point3d<F>(),
        math::normalized_vector3<F>::unit_x(),
        math::normalized_vector3<F>::unit_z(),
        math::vector2<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetExtends(F x, F y) { Rect.set_extends(x, y); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual math::point3d<F> SampleRandomPoint(math::vector3<F>& outN, F& outPDF) override;
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::rect<F> Rect;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldNormal;
    math::vector3<F> mWorldTagent;
};

struct SceneDisk : SceneObject
{
    SceneDisk() : Disk(math::point3d<F>(), math::normalized_vector3<F>::unit_x(), F(1)) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetRadius(F r) { Disk.set_radius(r); }
    void SetDualFace(bool dual) { mDualFace = dual; }
    virtual bool IsDualface() const override { return mDualFace; }
private:
    bool mDualFace = false;
    math::disk<F> Disk;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldNormal;
};

struct SceneCube : SceneObject
{
    SceneCube() : Cube(math::point3d<F>(), math::vector3<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray, F error) const override;
    void SetExtends(F x, F y, F z) { Cube.set_extends(x, y, z); }
private:
    math::cube<F> Cube;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldAxisX;
    math::vector3<F> mWorldAxisY;
    math::vector3<F> mWorldAxisZ;
};

class Scene
{
public:
    virtual ~Scene();
    void UpdateWorldTransform();
    HitRecord DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject, F epsilon);
    void Create(F aspect);
    int GetLightCount() const { return (int)mSceneLights.size(); }
    SceneObject* GetLightSourceByIndex(int index) { return mSceneLights[index]; }

private:
    virtual void CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects);
    void FindAllLights();
    std::vector<SceneObject*> mSceneObjects;
    std::vector<SceneObject*> mSceneLights;
};

class SimpleScene : public Scene
{
    virtual void CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects) override;
};

class SimpleBackCamera
{
    struct DegreeClampHelper
    {
        DegreeClampHelper(math::degree<F> degree)
            : value(math::clamp(degree.value, 1.0f, 179.0f)) { }
        const F value;
    };
public:
    SimpleBackCamera(math::degree<F> verticalFov);
    math::point3d<F> Position;
    const math::radian<F> HalfVerticalFov;
    const F HalfVerticalFovTangent;
};

class LitRenderer
{
public:
    LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void InitialSceneTransforms();
    void GenerateImageProgressive();
    bool NeedUpdate();

private:
    void GenerateSamples();
    void ResolveSamples();

    struct Sample
    {
        math::ray3d<F> ray;
        int pixelRow, pixelCol;
    };

    const int mMaxSampleCount = 4096;
    const int mSampleArrayCount;
    math::vector3<F> mClearColor;
    RenderCanvas mCanvas;
    SimpleBackCamera mCamera;
    std::unique_ptr<Scene> mScene;

    Sample* mImageSamples = nullptr;
};
