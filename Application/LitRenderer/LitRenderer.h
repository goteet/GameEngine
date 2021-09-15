#pragma once
#include <vector>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>

using F = double;
class RenderCanvas
{
public:
    RenderCanvas(unsigned char *canvasDataPtr, int width, int height, int linePitch);
    ~RenderCanvas();

    bool NeedUpdate();

    math::vector3<F>* GetBackbufferPtr() { return mBackbuffer; }
    bool NeedFlushBackbuffer = true;
    const int CanvasWidth;
    const int CanvasHeight;
    const int CanvasLinePitch;

private:
    void FlushLinearColorToGammaCorrectedCanvasData();

    bool mNeedUpdateWindowRect = false;
    unsigned char *mOutCanvasDataPtr;
    math::vector3<F>* mBackbuffer = nullptr;
};

struct SceneObject;

struct IntersectingInfo
{
    IntersectingInfo() = default;
    IntersectingInfo(SceneObject* o, math::vector3<F> n, F d)
        : Object(o), SurfaceNormal(n), Distance(d) { }

    SceneObject* Object = nullptr;
    math::vector3<F> SurfaceNormal;
    F Distance = F(0);
};

struct Light
{
    Light() = default;
    math::vector3<F> Position = math::vector3<F>::zero();
    math::vector3<F> Color = math::vector3<F>::zero();
    F Intensity = 1.0;
};

struct SceneObject
{
    virtual ~SceneObject() { }
    virtual IntersectingInfo IntersectWithRay(const math::ray3d<F>& ray) const = 0;
};


struct SceneSphere : SceneObject
{
    SceneSphere() : Sphere(math::point3d<F>(), 1) { }
    virtual IntersectingInfo IntersectWithRay(const math::ray3d<F>& ray) const override;
    math::sphere<F> Sphere;
};

struct ScenePlane : SceneObject
{
    ScenePlane() : Plane(math::point3d<F>(), math::vector3<F>::unit_x()) { }
    virtual IntersectingInfo IntersectWithRay(const math::ray3d<F>& ray) const override;
    math::plane<F> Plane;
};

struct SceneCube : SceneObject
{
    SceneCube() : Cube(math::point3d<F>(), math::vector3<F>::one()) { }
    virtual IntersectingInfo IntersectWithRay(const math::ray3d<F>& ray) const override;
    math::cube<F> Cube;
};

class Scene
{
public:
    Scene();
    ~Scene();

    IntersectingInfo DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject);

    unsigned int GetLightCount() const { return (unsigned int)mLights.size(); }

    const Light&  GetLightByIndex(unsigned int index) const;

    math::vector3<F> AmbientColor;

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
    LitRenderer(unsigned char * canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void GenerateImage();
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

    std::vector<Sample>* mImageSamples = nullptr;
};