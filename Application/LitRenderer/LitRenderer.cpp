#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"

const int MaxRecursiveDepth = 5;
const F RayBias = math::EPSILON<F> * 2.0f;

bool Trace(Scene& scene, const math::ray3d<F>& ray, math::vector3<F>& outSpectral, const SceneObject* excludeObject, int recursiveDepth)
{
    if (recursiveDepth == MaxRecursiveDepth)
        return false;

    IntersectingInfo objInfo = scene.DetectIntersecting(ray, nullptr);
    if (objInfo.Object == nullptr)
    {
        return false;
    }

    math::point3d<F> intersetionWorldPos = ray.calc_offset(objInfo.Distance);

    math::ray3d<F> shadowRay;
    shadowRay.set_origin(intersetionWorldPos + objInfo.SurfaceNormal * RayBias);

    for (int lightIndex = 0, lightCount = scene.GetLightCount(); lightIndex < lightCount; ++lightIndex)
    {
        const Light& light = scene.GetLightByIndex(lightIndex);

        math::vector3<F> towardLight = light.Position - intersetionWorldPos;
        F lightDistanceSqr = math::magnitude_sqr(towardLight);
        normalize(towardLight);
        shadowRay.set_direction(towardLight, math::norm);

        IntersectingInfo shadowInfo = scene.DetectIntersecting(shadowRay, objInfo.Object);
        if (shadowInfo.Object == nullptr || (shadowInfo.Distance * shadowInfo.Distance) > lightDistanceSqr)
        {
            //Lambertion
            F NdotL = math::dot(objInfo.SurfaceNormal, towardLight);
            NdotL = math::max2(NdotL, 0);

            F attenuation = light.Intensity / lightDistanceSqr;
            math::vector3<F> diffuseTerm = light.Color * NdotL * attenuation;
            outSpectral += scene.AmbientColor + diffuseTerm;
        }
        else
        {
            //in shadow.
            outSpectral += scene.AmbientColor;
        }
    }

    return true;
}


template<typename value_type>
value_type LinearToGammaCorrected22(value_type value)
{
    const value_type DisplayGamma = value_type(2.2);
    const value_type InvDisplayGamma = value_type(1.0) / DisplayGamma;
    const value_type Inv2_4 = value_type(1.0) / value_type(2.4);

    if (value < value_type(0))
    {
        return value_type(0);
    }
    else if (value <= value_type(0.0031308))
    {
        return value_type(12.92) * value;
    }
    else if (value < value_type(1.0))
    {
        return value_type(1.055) * pow(value, Inv2_4) - value_type(0.055);
    }
    else
    {
        return pow(value, InvDisplayGamma);
    }
}


RenderCanvas::RenderCanvas(unsigned char *canvasDataPtr, int width, int height, int linePitch)
    : mOutCanvasDataPtr(canvasDataPtr)
    , CanvasWidth(width)
    , CanvasHeight(height)
    , CanvasLinePitch(linePitch)
{
    mBackbuffer = new math::vector3<F>[CanvasWidth * CanvasHeight];

    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int pixelIndex = colIndex + rowIndex * CanvasWidth;
            mBackbuffer[pixelIndex].set(F(0.5), F(0.5), F(0.5));
        }
    }
}

RenderCanvas::~RenderCanvas()
{
    SafeDeleteArray(mBackbuffer);
}

bool RenderCanvas::NeedUpdate()
{
    if (NeedFlushBackbuffer)
    {
        FlushLinearColorToGammaCorrectedCanvasData();
        if (!NeedFlushBackbuffer)
        {
            mNeedUpdateWindowRect = true;
        }
    }
    return mNeedUpdateWindowRect;
}

void RenderCanvas::FlushLinearColorToGammaCorrectedCanvasData()
{
    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        int bufferRowOffset = rowIndex * CanvasWidth;
        int canvasRowOffset = rowIndex * CanvasLinePitch;
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int bufferIndex = colIndex + bufferRowOffset;
            int canvasIndex = colIndex * 3 + canvasRowOffset;

            const math::vector3<F>& color = mBackbuffer[bufferIndex];
            for (int compIndex = 2; compIndex >= 0; compIndex--)
            {
                F c = LinearToGammaCorrected22(color.v[compIndex]);
                c = math::clamp(c, F(0.0), F(1.0) - math::EPSILON<F>);
                mOutCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(c * F(256.0));
            }
        }
    }
    NeedFlushBackbuffer = false;
}

LitRenderer::LitRenderer(unsigned char * canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvas(canvasDataPtr, canvasWidth, canvasHeight, canvasLinePitch)
    , mClearColor(0, 0, 0)
    , mCamera(math::degree<F>(60))
    , mSampleArrayCount(canvasWidth * canvasHeight)

{
    mImageSamples = new std::vector<Sample>[canvasWidth * canvasHeight];
}

LitRenderer::~LitRenderer()
{
    for (int index = 0; index < mSampleArrayCount; index++)
    {
        mImageSamples[index].clear();
    }
    SafeDeleteArray(mImageSamples);
}

void LitRenderer::GenerateImage()
{
    GenerateSamples();
    ResolveSamples();
    mCanvas.NeedFlushBackbuffer = true;
}

bool LitRenderer::NeedUpdate()
{
    return mCanvas.NeedUpdate();
}

void LitRenderer::GenerateSamples()
{
    const F PixelSize = F(0.1);
    const F HalfPixelSize = PixelSize * F(0.5);
    F halfWidth = mCanvas.CanvasWidth * F(0.5) * PixelSize;
    F halfHeight = mCanvas.CanvasHeight * F(0.5) * PixelSize;
    //     <---> (half height)
    //  +  o----.¡¡(o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  +  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    F cameraZ = halfHeight / mCamera.HalfVerticalFovTangent;
    mCamera.Position.z = -cameraZ;

    Sample sample;
    sample.ray.set_origin(mCamera.Position);


    auto canvasPositionToRay = [&cameraZ](F x, F y) -> math::vector3<F> {
        //vector3<float>(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::vector3<F>(x, y, cameraZ);
    };

    const F QuaterPixelSize = F(0.5) * HalfPixelSize;
    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            std::vector<Sample>& samples = mImageSamples[colIndex + rowOffset];

            sample.pixelCol = colIndex;
            sample.pixelRow = rowIndex;

            const F pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const F pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

            sample.ray.set_direction(canvasPositionToRay(pixelCenterX - QuaterPixelSize, pixelCenterY));
            samples.push_back(sample);

            sample.ray.set_direction(canvasPositionToRay(pixelCenterX + QuaterPixelSize, pixelCenterY));
            samples.push_back(sample);

            sample.ray.set_direction(canvasPositionToRay(pixelCenterX, pixelCenterY - QuaterPixelSize));
            samples.push_back(sample);

            sample.ray.set_direction(canvasPositionToRay(pixelCenterX, pixelCenterY + QuaterPixelSize));
            samples.push_back(sample);
        }
    }
}

void LitRenderer::ResolveSamples()
{
    const int StartRecursiveDepth = 0;
    math::vector3<F>* canvasDataPtr = mCanvas.GetBackbufferPtr();
    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            std::vector<Sample>& samples = mImageSamples[colIndex + rowOffset];
            math::vector3<F>& canvasPixel = canvasDataPtr[colIndex + rowOffset];

            const size_t sampleCount = samples.size();
            math::vector3<F> accSpectral(0, 0, 0);
            math::vector3<F> traceSpectral;
            int hittedSampleCount = 0;
            for (const Sample& sample : samples)
            {
                traceSpectral.set(0, 0, 0);
                if (Trace(mScene, sample.ray, traceSpectral, nullptr, StartRecursiveDepth))
                {
                    accSpectral += traceSpectral;
                    hittedSampleCount++;
                }

            }
            if (hittedSampleCount > 0)
            {
                accSpectral *= (F(1) / sampleCount);
            }
            else
            {
                accSpectral = mClearColor;
            }
            canvasPixel = accSpectral;
        }
    }
}

SimpleBackCamera::SimpleBackCamera(math::degree<F> verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value * F(0.5))
    , HalfVerticalFovTangent(math::tan(math::degree<F>(DegreeClampHelper(verticalFov).value * F(0.5))))
    , Position(0, 0, 0)
{ }


Scene::Scene()
    : AmbientColor(F(0.01), F(0.01), F(0.01))
{
    const F SceneDistance = 5;
    const F lSphereRadius = 10;
    const F rSphereRadius = lSphereRadius * F(0.5);
    const F LightHeight = lSphereRadius * 4;
    const F SceneSize = 40;
    const F GroundHeight = -SceneSize;

    Light mainLight;
    mainLight.Position.set(0, GroundHeight + F(1.99) * SceneSize, SceneDistance);
    mainLight.Color.set(1, 1, 1);
    mainLight.Intensity = 400;

    mLights.push_back(mainLight);

    SceneSphere* lSphere = new SceneSphere();
    SceneSphere* rSphere = new SceneSphere();

    lSphere->Sphere.center().set(-2 - lSphereRadius, GroundHeight + lSphereRadius, SceneDistance);
    lSphere->Sphere.set_radius(lSphereRadius);

    rSphere->Sphere.center().set(+2 + rSphereRadius, GroundHeight + rSphereRadius, SceneDistance);
    rSphere->Sphere.set_radius(rSphereRadius);
    mSceneObjects.push_back(rSphere);
    mSceneObjects.push_back(lSphere);

    ScenePlane* lWall = new ScenePlane();
    ScenePlane* rWall = new ScenePlane();
    ScenePlane* tWall = new ScenePlane();
    ScenePlane* bWall = new ScenePlane();
    ScenePlane* fWall = new ScenePlane();

    lWall->Plane.position().set(-SceneSize, 0, 0);
    lWall->Plane.set_normal(math::vector3<F>::unit_x(), math::norm);

    rWall->Plane.position().set(SceneSize, 0, 0);
    rWall->Plane.set_normal(math::vector3<F>::unit_x_neg(), math::norm);

    tWall->Plane.position().set(0, GroundHeight + 2 * SceneSize, 0);
    tWall->Plane.set_normal(math::vector3<F>::unit_y_neg(), math::norm);

    bWall->Plane.position().set(0, GroundHeight, 0);
    bWall->Plane.set_normal(math::vector3<F>::unit_y(), math::norm);

    fWall->Plane.position().set(0, 0, F(1.5) * SceneSize);
    fWall->Plane.set_normal(math::vector3<F>::unit_z_neg(), math::norm);

    mSceneObjects.push_back(lWall);
    mSceneObjects.push_back(rWall);
    mSceneObjects.push_back(tWall);
    mSceneObjects.push_back(bWall);
    mSceneObjects.push_back(fWall);
}

Scene::~Scene()
{
    for (const SceneObject* obj : mSceneObjects)
    {
        SafeDelete(obj);
    }
    mSceneObjects.clear();
}

IntersectingInfo Scene::DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject)
{
    IntersectingInfo result;
    for (const SceneObject* obj : mSceneObjects)
    {
        if (excludeObject != obj)
        {
            IntersectingInfo info = obj->IntersectWithRay(ray);
            if (info.Object != nullptr)
            {
                if (result.Object == nullptr || result.Distance > info.Distance)
                {
                    result = info;
                }
            }
        }
    }
    return result;
}

const Light & Scene::GetLightByIndex(unsigned int index) const
{
    static Light l;
    if (index >= GetLightCount())
    {
        return l;
    }
    else
    {
        return mLights[index];
    }
}

IntersectingInfo SceneSphere::IntersectWithRay(const math::ray3d<F>& ray) const
{
    F t0, t1;
    math::intersection result = math::intersect(ray, Sphere, t0, t1);
    if (result == math::intersection::none)
    {
        return IntersectingInfo();
    }

    if (result == math::intersection::inside)
    {
        t0 = t1;
    }

    math::point3d<F> intersectPosition = ray.calc_offset(t0);
    math::vector3<F> surfaceNormal = normalized(intersectPosition - Sphere.center());

    return IntersectingInfo(const_cast<SceneSphere*>(this), surfaceNormal, t0);
}

IntersectingInfo ScenePlane::IntersectWithRay(const math::ray3d<F>& ray) const
{
    F t;
    math::intersection result = math::intersect(ray, Plane, t);
    if (result == math::intersection::none)
    {
        return IntersectingInfo();
    }
    else
    {
        return IntersectingInfo(const_cast<ScenePlane*>(this), Plane.normal(), t);
    }
}