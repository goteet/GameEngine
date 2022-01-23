#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"

const int MaxRecursiveDepth = 10;
const F RayBias = math::EPSILON<F> *50.0f;

#include <random>
template<typename T>
struct random
{
    static random<T>& instance() { static random<T> instance; return instance; }

    static T value() { return instance()._value(); }

private:
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<F> distribution;
    random() : generator(randomDevice()), distribution(F(0), F(1)) { }
    F _value() { return distribution(generator); }
};


math::vector3<F> GenerateUnitSphereVector()
{
    while (true) {
        F x = F(2) * random<F>::value() - F(1);
        F y = F(2) * random<F>::value() - F(1);
        F z = F(2) * random<F>::value() - F(1);
        math::vector3<F> v = math::vector3<F>(x, y, z);
        if (math::magnitude_sqr(v) < F(1))
        {
            return v;
        }
    }
}



math::vector3<F> GenerateHemisphereDirection(const math::vector3<F>& normal)
{
    F cosTheta = F(1) - F(2) * random<F>::value();
    F sinTheta = sqrt(F(1) - cosTheta * cosTheta);
    math::radian<F> phi(F(2) * math::PI<F> *random<F>::value());
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;
    math::vector3<F> unit(x, y, z);

    return math::dot(unit, normal) >= 0 ? unit : -unit;
}

bool Lambertian::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation)
{
    math::vector3<F> Direction = GenerateHemisphereDirection(N);
    outScattering.set_origin(P);
    outScattering.set_direction(Direction, math::norm);
    outAttenuation = Albedo;
    return true;
}

math::vector3<F> Reflect(const math::vector3<F>& In, const math::vector3<F>& N)
{
    return In - F(2) * math::dot(In, N) * N;
}

bool Metal::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation)
{
    math::vector3<F> FuzzyDirection = Fuzzy * GenerateUnitSphereVector();
    math::vector3<F> Direction = Reflect(Ray.direction(), N) + FuzzyDirection;
    outScattering.set_origin(P);
    outScattering.set_direction(Direction);
    outAttenuation = Albedo;
    return math::dot(Direction, N) > F(0);
}

F Power5(F Base)
{
    F Ret = Base * Base;
    return Ret * Ret * Base;
}

F ReflectanceSchlick(F CosTheta, F RefractionRatio)
{
    // Use Schlick's approximation for reflectance.
    F F0 = (1 - RefractionRatio) / (1 + RefractionRatio);
    F F0Sqr = F0 * F0;
    F Base = F(1) - CosTheta;
    return F0Sqr + (F(1) - F0Sqr) * Power5(Base);
}

bool Dielectric::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering, math::vector3<F>& outAttenuation)
{
    const F AirRefractiveIndex = F(1.0003);

    const math::vector3<F>& InDirection = Ray.direction();

    const F RefractionRatio = IsFrontFace ? (AirRefractiveIndex / RefractiveIndex) : (RefractiveIndex / AirRefractiveIndex);
    const F IdotN = -math::dot(InDirection, N);
    F CosTheta = math::clamp(IdotN);
    F SinThetaSqr = math::clamp(F(1) - CosTheta * CosTheta);
    F Det = F(1) - SinThetaSqr * RefractionRatio * RefractionRatio;
    bool RefractRay = Det >= F(0);
    bool ReflectRay = ReflectanceSchlick(CosTheta, RefractionRatio) > random<F>::value();
    math::vector3<F> ScatteredDirection;
    if (!RefractRay || ReflectRay)
    {
        ScatteredDirection = Reflect(InDirection, N);
    }
    else
    {
        math::vector3<F> Rprep = RefractionRatio * (InDirection + CosTheta * N);
        math::vector3<F> Rpall = N * sqrt(Det);
        ScatteredDirection = Rprep - Rpall;
    }

    outScattering.set_origin(P);
    outScattering.set_direction(ScatteredDirection);

    outAttenuation = math::vector3<F>::one();
    return true;
}

math::vector3<F> Trace(Scene& scene, const math::ray3d<F>& ray, int recursiveDepth)
{
    const F Epsilon = RayBias;//F(0.001);
    if (recursiveDepth == MaxRecursiveDepth)
    {
        return math::vector3<F>::zero();
    }

    //return Sky Color if there is no collision occured In the direction.
    HitRecord contactRecord = scene.DetectIntersecting(ray, nullptr);
    if (contactRecord.Object == nullptr)
    {
        F t = math::clamp(ray.direction().y * F(0.5) + F(0.25), F(0), F(1));
        return math::lerp(math::vector3<F>::one(), math::vector3<F>(F(0.5), F(0.7), F(1)), t);
    }

    //Collision occurred.
    F biasedDistance = math::max2<F>(contactRecord.Distance - Epsilon, F(0));
    math::point3d<F> surfacePoint = ray.calc_offset(biasedDistance);
    math::vector3<F> attenuation;
    math::ray3d<F> scattering;
    if (contactRecord.Object->Material->Scattering(surfacePoint, contactRecord.SurfaceNormal, ray, contactRecord.IsFrontFace, scattering, attenuation))
    {
        scattering.set_origin(scattering.origin() + scattering.direction() * Epsilon);
        return attenuation * Trace(scene, scattering, recursiveDepth + 1);
    }

    return math::vector3<F>::zero();
}

namespace Legacy
{
    //    bool Trace(Scene& scene, const math::ray3d<F>& ray, math::vector3<F>& outSpectral, const SceneObject* excludeObject, int recursiveDepth)
    //    {
    //        if (recursiveDepth == MaxRecursiveDepth)
    //            return false;
    //
    //        HitRecord contactSurfaceInfo = scene.DetectIntersecting(ray, nullptr);
    //        if (contactSurfaceInfo.Object == nullptr)
    //        {
    //            return false;
    //        }
    //
    //        math::point3d<F> intersetionWorldPos = ray.calc_offset(contactSurfaceInfo.Distance);
    //
    //        math::ray3d<F> shadowRay;
    //        shadowRay.set_origin(intersetionWorldPos + contactSurfaceInfo.SurfaceNormal * RayBias);
    //
    //        //Pucture Reflectance Equation.
    //        for (int lightIndex = 0, lightCount = scene.GetLightCount(); lightIndex < lightCount; ++lightIndex)
    //        {
    //            const Light& light = scene.GetLightByIndex(lightIndex);
    //            bool isDirectional = light.LightType == ELightType::Directional;
    //
    //            math::vector3<F> towardLight; F lightDistanceSqr = std::numeric_limits<F>::max();
    //            if (isDirectional)
    //            {
    //                towardLight = -light.Position;
    //            }
    //            else
    //            {
    //                towardLight = light.Position - intersetionWorldPos;
    //                lightDistanceSqr = math::magnitude_sqr(towardLight);
    //                normalize(towardLight);
    //            }
    //
    //            shadowRay.set_direction(towardLight, math::norm);
    //            HitRecord shadowInfo = scene.DetectIntersecting(shadowRay, contactSurfaceInfo.Object);
    //            if (shadowInfo.Object == nullptr || (shadowInfo.Distance * shadowInfo.Distance) > lightDistanceSqr)
    //            {
    //                const math::vector3<F>& N = contactSurfaceInfo.SurfaceNormal;
    //                const math::vector3<F>& L = towardLight;
    //
    //                //Lambertion BRDF diffuse
    //                const IMaterial& material = *contactSurfaceInfo.Object->Material;
    //                math::vector3<F> BRDF = material.Albedo; //we ignore 1/PI.
    //
    //                F NdotL = math::max2(math::dot(N, L), 0);
    //
    //                F attenuation = isDirectional ? F(1) : (F(1) / lightDistanceSqr);
    //                math::vector3<F> Li = light.Intensity * attenuation * light.Color;//we ignore PI here too...
    //
    //                math::vector3<F> Lo = BRDF * Li * NdotL;
    //                outSpectral += Lo;
    //            }
    //            else
    //            {
    //                //In shadow.
    //            }
    //        }
    //        return true;
    //    }
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


RenderCanvas::RenderCanvas(unsigned char* canvasDataPtr, int width, int height, int linePitch)
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
            mBackbuffer[pixelIndex].set(F(0.0), F(0.0), F(0.0));
        }
    }
}

RenderCanvas::~RenderCanvas()
{
    SafeDeleteArray(mBackbuffer);
}

bool RenderCanvas::NeedUpdate(int SampleBatchCount)
{
    if (NeedFlushBackbuffer)
    {
        FlushLinearColorToGammaCorrectedCanvasData(SampleBatchCount);
        if (!NeedFlushBackbuffer)
        {
            mNeedUpdateWindowRect = true;
        }
    }
    return mNeedUpdateWindowRect;
}

void RenderCanvas::FlushLinearColorToGammaCorrectedCanvasData(int SampleBatchCount)
{
    F invSampleBatchCount = F(1) / F(SampleBatchCount);
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
                F c = LinearToGammaCorrected22(color.v[compIndex] * invSampleBatchCount);
                c = math::clamp(c, F(0.0), F(1.0) - math::EPSILON<F>);
                mOutCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(c * F(256.0));
            }
        }
    }
    NeedFlushBackbuffer = false;
}

LitRenderer::LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvas(canvasDataPtr, canvasWidth, canvasHeight, canvasLinePitch)
    , mClearColor(0, 0, 0)
    , mCamera(math::degree<F>(50))
    , mSampleArrayCount(canvasWidth* canvasHeight)
    , mScene(F(canvasWidth) / F(canvasHeight))

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

void LitRenderer::InitialSceneTransforms()
{
    mScene.UpdateWorldTransform();
}

void LitRenderer::GenerateImageProgressive()
{
    if (mSampleBatchCount < 20000)
    {
        GenerateSamples();
        ResolveSamples();
        mCanvas.NeedFlushBackbuffer = true;
        mSampleBatchCount += 1;
    }
}

bool LitRenderer::NeedUpdate()
{
    return mCanvas.NeedUpdate(mSampleBatchCount);
}

void LitRenderer::GenerateSamples()
{
    const F PixelSize = F(0.1);
    const F HalfPixelSize = PixelSize * F(0.5);
    F halfWidth = mCanvas.CanvasWidth * F(0.5) * PixelSize;
    F halfHeight = mCanvas.CanvasHeight * F(0.5) * PixelSize;
    //     <---> (half height)
    //  .  o----. (o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  .  |/
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
            samples.clear();

            sample.pixelCol = colIndex;
            sample.pixelRow = rowIndex;

            const F pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const F pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

            F x = F(2) * random<F>::value() - F(1);
            F y = F(2) * random<F>::value() - F(1);

            sample.ray.set_direction(canvasPositionToRay(pixelCenterX + x * HalfPixelSize, pixelCenterY + y * HalfPixelSize));
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
                accSpectral += Trace(mScene, sample.ray, StartRecursiveDepth);
                hittedSampleCount++;

            }
            if (hittedSampleCount > 0)
            {
                accSpectral *= (F(1) / sampleCount);
            }
            else
            {
                accSpectral = mClearColor;
            }
            canvasPixel += accSpectral;
        }
    }
}

SimpleBackCamera::SimpleBackCamera(math::degree<F> verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value* F(0.5))
    , HalfVerticalFovTangent(math::tan(math::degree<F>(DegreeClampHelper(verticalFov).value* F(0.5))))
    , Position(0, 0, 0)
{ }

Scene::Scene(F aspect)
{
    const F SceneSize = 60;
    const F SceneNear = 1;
    const F SceneFar = SceneSize * F(2.5);
    const F SceneBottom = -SceneSize;
    const F SceneTop = SceneSize;
    const F SceneLeft = -SceneSize * aspect;
    const F SceneRight = SceneSize * aspect;

    const F SceneCenterX = (SceneLeft + SceneRight) * F(0.5);
    const F SceneCenterY = (SceneBottom + SceneTop) * F(0.5);
    const F SceneCenterZ = (SceneNear + SceneFar) * F(0.5);
    const F SceneExtendX = (SceneRight - SceneLeft) * F(0.5);
    const F SceneExtendY = (SceneTop - SceneBottom) * F(0.5);
    const F SceneExtendZ = (SceneFar - SceneNear) * F(0.5);

    const F SceneDistance = 5;
    const F SmallObjectSize = 8;
    const F BigObjectSize = SmallObjectSize * F(1.75);

    Light mainLight;
    mainLight.Position.set(SceneCenterX, SceneTop - F(1), SceneCenterZ);
    mainLight.Color.set(1, 1, 1);
    mainLight.Intensity = SceneExtendY * 2 * 5;
    mainLight.LightType = ELightType::Puncture;
    mLights.push_back(mainLight);

    Light directionalLight;
    directionalLight.Position.set(F(-0.1), F(-0.1), 1);
    directionalLight.Color.set(1, 1, 1);
    directionalLight.Intensity = F(0.075);
    directionalLight.LightType = ELightType::Directional;
    normalize(directionalLight.Position);
    mLights.push_back(directionalLight);

    SceneSphere* lSphere = new SceneSphere(); mSceneObjects.push_back(lSphere);
    lSphere->SetRadius(SmallObjectSize);
    lSphere->SetTranslate(
        SceneCenterX - 20,
        SceneBottom + SmallObjectSize,
        SceneCenterZ - 5);
    lSphere->Material = std::make_unique<Lambertian>();
    
    SceneSphere* metalSphere = new SceneSphere(); mSceneObjects.push_back(metalSphere);
    metalSphere->SetRadius(16);
    metalSphere->SetTranslate(
        SceneCenterX,
        SceneCenterY,
        SceneCenterZ + 10);
    metalSphere->Material = std::make_unique<Metal>(F(1.0), F(0.85), F(0.45), F(0.08));

    SceneSphere* dielectricSphereFloat = new SceneSphere(); mSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(15);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 40,
        SceneCenterY,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material = std::make_unique<Dielectric>(F(1.5));


    SceneSphere* dielectricSphere = new SceneSphere(); mSceneObjects.push_back(dielectricSphere);
    dielectricSphere->SetRadius(20);
    dielectricSphere->SetTranslate(
        SceneLeft + 30,
        SceneBottom + 20,
        SceneFar - 30);
    dielectricSphere->Material = std::make_unique<Dielectric>(F(1.4));

    SceneCube* lCube = new SceneCube(); mSceneObjects.push_back(lCube);
    lCube->SetExtends(SmallObjectSize, BigObjectSize, SmallObjectSize);
    lCube->SetTranslate(
        SceneCenterX + 20 + BigObjectSize,
        SceneBottom + BigObjectSize,
        SceneCenterZ + 30);
    lCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(-30)));
    lCube->Material = std::make_unique<Lambertian>();
    
    SceneCube* rCube = new SceneCube(); mSceneObjects.push_back(rCube);
    rCube->SetExtends(SmallObjectSize, SmallObjectSize, SmallObjectSize);
    rCube->SetTranslate(
        SceneCenterX + 15 + SmallObjectSize,
        SceneBottom + SmallObjectSize,
        SceneCenterZ + 5);
    rCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(60)));
    rCube->Material = std::make_unique<Lambertian>();
    
    SceneRect* wallLeft = new SceneRect(); mSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));
    
    SceneRect* wallRight = new SceneRect(); mSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

    SceneRect* wallTop = new SceneRect(); mSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

    SceneRect* wallBottom = new SceneRect(); mSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
    wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2));

    SceneRect* wallFar = new SceneRect(); mSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
    wallFar->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));
}

Scene::~Scene()
{
    for (const SceneObject* obj : mSceneObjects)
    {
        SafeDelete(obj);
    }
    mSceneObjects.clear();
}

void Scene::UpdateWorldTransform()
{
    for (auto* object : mSceneObjects)
    {
        object->UpdateWorldTransform();
    }
}

HitRecord Scene::DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject)
{
    HitRecord result;
    for (const SceneObject* obj : mSceneObjects)
    {
        if (excludeObject != obj)
        {
            HitRecord info = obj->IntersectWithRay(ray);
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

const Light& Scene::GetLightByIndex(unsigned int index) const
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

void Transform::UpdateWorldTransform()
{
    TransformMatrix = math::matrix4x4<F>::TR(Translate, Rotation);
}

void SceneObject::UpdateWorldTransform()
{
    Transform.UpdateWorldTransform();
}

void SceneSphere::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldCenter = transform(Transform.TransformMatrix, mSphere.center());
}

HitRecord SceneSphere::IntersectWithRay(const math::ray3d<F>& ray) const
{
    F t0, t1;
    bool front = true;
    math::intersection result = math::intersect_sphere(ray, mWorldCenter, mSphere.radius_sqr(), t0, t1);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else if (result == math::intersection::inside)
    {
        t0 = t1;
        front = false;
    }

    math::point3d<F> intersectPosition = ray.calc_offset(t0);
    math::vector3<F> surfaceNormal = normalized(intersectPosition - mWorldCenter);
    return HitRecord(const_cast<SceneSphere*>(this), front, (front ? surfaceNormal : -surfaceNormal), t0);
}

void SceneRect::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Rect.position());
    mWorldNormal = transform(Transform.TransformMatrix, Rect.normal()); // no scale so there...
    mWorldTagent = transform(Transform.TransformMatrix, Rect.tangent());
}

HitRecord SceneRect::IntersectWithRay(const math::ray3d<F>& ray) const
{
    F t;
    bool front = true;
    math::intersection result = math::intersect_rect(ray, mWorldPosition, mWorldNormal, mWorldTagent, Rect.extends(), mDualFace, t);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else
    {
        front = math::dot(mWorldNormal, ray.direction()) < 0;
        return HitRecord(const_cast<SceneRect*>(this), front, front ? mWorldNormal : -mWorldNormal, t);
    }
}

void SceneCube::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();
    mWorldPosition = transform(Transform.TransformMatrix, Cube.center());
    mWorldAxisX = transform(Transform.TransformMatrix, Cube.axis_x());
    mWorldAxisY = transform(Transform.TransformMatrix, Cube.axis_y());
    mWorldAxisZ = transform(Transform.TransformMatrix, Cube.axis_z());
}

HitRecord SceneCube::IntersectWithRay(const math::ray3d<F>& ray) const
{
    F t0, t1;
    bool front = true;

    math::vector3<F> n0, n1;
    math::intersection result = math::intersect_cube(ray, mWorldPosition,
        mWorldAxisX, mWorldAxisY, mWorldAxisZ,
        Cube.width(), Cube.height(), Cube.depth(),
        t0, t1, n0, n1);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else if (result == math::intersection::inside)
    {
        n0 = -n1;
        t0 = t1;
        front = false;
    }

    return HitRecord(const_cast<SceneCube*>(this), front, n0, t0);
}
