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

    bool NeedUpdate(int SampleBatchCount);

    math::vector3<F>* GetBackbufferPtr() { return mBackbuffer; }
    bool NeedFlushBackbuffer = true;
    const int CanvasWidth;
    const int CanvasHeight;
    const int CanvasLinePitch;

private:
    void FlushLinearColorToGammaCorrectedCanvasData(int SampleBatchCount);

    bool mNeedUpdateWindowRect = false;
    unsigned char* mOutCanvasDataPtr;
    math::vector3<F>* mBackbuffer = nullptr;
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

enum class ELightType
{
    Directional, Puncture
};

struct Light
{
    Light() = default;
    ELightType LightType = ELightType::Puncture;
    math::vector3<F> Position = math::vector3<F>::zero();
    math::vector3<F> Color = math::vector3<F>::zero();
    F Intensity = 1.0;
};

struct IMaterial
{
    virtual ~IMaterial() { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation) = 0;
};

struct Lambertian : public IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();

    Lambertian() = default;
    Lambertian(F r, F g, F b) : Albedo(r, g, b) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation) override;
};

struct Metal : public IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();
    F Fuzzy = F(0);

    Metal() = default;
    Metal(F r, F g, F b, F f) : Albedo(r, g, b), Fuzzy(f) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation) override;
};

struct Dielectric : public IMaterial
{
    F RefractiveIndex = F(1.0003);

    Dielectric() = default;
    Dielectric(F ior) : RefractiveIndex(ior) { }
    virtual bool Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation) override;
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
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray) const = 0;
    void SetTranslate(F x, F y, F z) { Transform.Translate.set(x, y, z); }
    void SetRotation(const math::quaternion<F>& q) { Transform.Rotation = q; }

    Transform Transform;
    std::unique_ptr<IMaterial> Material;
};


struct SceneSphere : SceneObject
{
    SceneSphere() : mSphere(math::point3d<F>(), 1) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray) const override;
    void SetRadius(F radius) { mSphere.set_radius(radius); }
private:
    math::sphere<F> mSphere;
    math::point3d<F> mWorldCenter;
};

struct SceneRect : SceneObject
{
    SceneRect() : Rect(math::point3d<F>(),
        math::vector3<F>::unit_x(), math::norm,
        math::vector3<F>::unit_z(), math::norm,
        math::vector2<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray) const override;
    void SetExtends(F x, F y) { Rect.set_extends(x, y); }
    void SetDualFace(bool dual) { mDualFace = dual; }
private:
    bool mDualFace = false;
    math::rect<F> Rect;
    math::point3d<F> mWorldPosition;
    math::vector3<F> mWorldNormal;
    math::vector3<F> mWorldTagent;
};

struct SceneCube : SceneObject
{
    SceneCube() : Cube(math::point3d<F>(), math::vector3<F>::one()) { }
    virtual void UpdateWorldTransform() override;
    virtual HitRecord IntersectWithRay(const math::ray3d<F>& ray) const override;
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
    Scene(F aspect);
    ~Scene();
    void UpdateWorldTransform();
    HitRecord DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject);
    unsigned int GetLightCount() const { return (unsigned int)mLights.size(); }
    const Light& GetLightByIndex(unsigned int index) const;
private:
    std::vector<Light> mLights;
    std::vector<SceneObject*> mSceneObjects;
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

    const int mSampleArrayCount;
    math::vector3<F> mClearColor;
    RenderCanvas mCanvas;
    SimpleBackCamera mCamera;
    Scene mScene;
    int mSampleBatchCount = 0;

    std::vector<Sample>* mImageSamples = nullptr;
};
