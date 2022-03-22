#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"


struct TerminalRecord
{
    static const int MaxRecursiveDepth = 10;

    math::vector3<F> OuterReflectance = math::vector3<F>::one();
    int CurrentRecursiveDepth = MaxRecursiveDepth;

    TerminalRecord Next(const math::vector3<F>& reflectance) const
    {
        return TerminalRecord{ OuterReflectance * reflectance,  CurrentRecursiveDepth - 1 };
    }

    bool IsTerminal() const
    {
        return CurrentRecursiveDepth == 0 || math::near_zero(OuterReflectance);
    };
};

math::vector3<F> Trace(Scene& scene, const math::ray3d<F>& ray, const TerminalRecord& TerminalRecord)
{
    if (TerminalRecord.IsTerminal())
    {
        return math::vector3<F>::zero();
    }

    HitRecord contactRecord = scene.DetectIntersecting(ray, nullptr, math::SMALL_NUM<F>);
    if (contactRecord.Object == nullptr)
    {
        return math::vector3<F>::zero();
    }

    //Collision occurred.
    const SceneObject& shadingObject = *contactRecord.Object;
    const IMaterial& material = *shadingObject.Material;
    const math::vector3<F>& normal = contactRecord.SurfaceNormal;
    F biasedDistance = math::max2<F>(contactRecord.Distance, F(0));
    math::point3d<F> shadingPoint = ray.calc_offset(biasedDistance);

    math::vector3<F> f = math::vector3<F>::zero();
    math::ray3d<F> scattering;

    int numLights = scene.GetLightCount();
    F invNumLights = F(1) / (numLights + F(1));
    F ratioLights = numLights * invNumLights;
    F epsilon = random<F>::value();
    bool result = false;
    bool chooseLightSample = epsilon <= ratioLights;
    if (chooseLightSample)
    {
        int lightIndex = math::floor2<int>(epsilon / invNumLights);
        lightIndex = math::clamp(lightIndex, 0, numLights - 1);
        SceneObject* LightObject = scene.GetLightSourceByIndex(lightIndex);

        math::vector3<F> lightN;
        math::point3d<F> LightSampleP = LightObject->SampleRandomPoint(lightN);
        math::vector3<F> lightDisplacement = LightSampleP - shadingPoint;

        //direct light sampling.
        scattering.set_origin(shadingPoint);
        scattering.set_direction(lightDisplacement);

        const math::normalized_vector3<F>& lightV = scattering.direction();
        F cosThetaPrime = math::dot(lightN, -lightV);

        if (cosThetaPrime < -math::SMALL_NUM<F> && LightObject->IsDualface())
        {
            cosThetaPrime = -cosThetaPrime;
        }

        f = material.Albedo / math::PI<F>;
        result = cosThetaPrime > math::SMALL_NUM<F>;
    }
    else
    {
        result = material.Scattering(shadingPoint, normal, ray, contactRecord.IsFrontFace, scattering, f);
    }

    if (result)
    {
        F pdfLight = F(0);
        for (int lightIndex = 0; lightIndex < numLights; lightIndex++)
        {
            SceneObject* light = scene.GetLightSourceByIndex(lightIndex);
            pdfLight += light->SamplePdf(scattering);
        }
        pdfLight = pdfLight / F(numLights);

        F pdfBxDF = material.pdf(normal, ray.direction(), scattering.direction());
        F pdf = ratioLights * pdfLight + (F(1) - ratioLights) * pdfBxDF;
        F cosTheta = math::dot(normal, scattering.direction());
        math::vector3<F> reflectance = f * cosTheta / pdf;
        math::vector3<F> Li = Trace(scene, scattering, TerminalRecord.Next(reflectance));

        math::vector3<F> emission = material.Emitting();
        math::vector3<F> reflection = f * Li * cosTheta / pdf;
        return emission + reflection;
    }
    else
    {
        math::vector3<F> emission = material.Emitting();
        return emission;
    }
}

math::point3d<F> SceneRect::SampleRandomPoint(math::vector3<F>& outN) const
{
    outN = mWorldNormal;

    F e1 = random<F>::value() * Rect.width();
    F e2 = random<F>::value() * Rect.height();

    math::vector3<F> Bitangent = math::cross(mWorldNormal, mWorldTagent);
    return math::point3d<F>(mWorldPosition + e1 * mWorldTagent + e2 * Bitangent);
}

F SceneRect::SamplePdf(const math::ray3d<F>& ray) const
{
    HitRecord hr = this->IntersectWithRay(ray, math::SMALL_NUM<F>);
    if (hr.Object != this)
    {
        return F(0);
    }

    //calculate pdf(w) = pdf(x') * dist_sqr / cos_theta'
    // pdf(x') = 1 / area = > pdf(w) = dist_sqr / (area * cos_theta')
    const math::normalized_vector3<F>& V = ray.direction();
    const math::normalized_vector3<F>& N = this->mWorldNormal;
    F cosThetaPrime = math::dot(N, -V);

    if (cosThetaPrime < -math::SMALL_NUM<F> && IsDualface())
    {
        cosThetaPrime = -cosThetaPrime;
    }

    if (cosThetaPrime <= math::SMALL_NUM<F>)
    {
        return F(0);
    }

    F area = Rect.width() * Rect.height();
    return (hr.Distance * hr.Distance) / (area * cosThetaPrime);

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
    F invSampleCout = F(1) / mSampleCount;
    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        int bufferRowOffset = rowIndex * CanvasWidth;
        int canvasRowOffset = rowIndex * CanvasLinePitch;
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int bufferIndex = colIndex + bufferRowOffset;
            int canvasIndex = colIndex * 3 + canvasRowOffset;

            const math::vector3<F>& buffer = mBackbuffer[bufferIndex];
            for (int compIndex = 2; compIndex >= 0; compIndex--)
            {
                F c = LinearToGammaCorrected22(buffer[compIndex] * invSampleCout);
                c = math::clamp(c, F(0.0), F(0.9999));
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
    , mScene(std::make_unique<SimpleScene>())

{
    mScene->Create(F(canvasWidth) / F(canvasHeight));
    mImageSamples = new Sample[canvasWidth * canvasHeight];
}

LitRenderer::~LitRenderer()
{
    SafeDeleteArray(mImageSamples);
}

void LitRenderer::InitialSceneTransforms()
{
    mScene->UpdateWorldTransform();
}

void LitRenderer::GenerateImageProgressive()
{
    if (mCanvas.GetmSampleCount() <= mMaxSampleCount || mMaxSampleCount <= 0)
    {
        GenerateSamples();
        ResolveSamples();
        mCanvas.NeedFlushBackbuffer = true;
    }
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
    //  .  o----. (o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  .  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    F cameraZ = halfHeight / mCamera.HalfVerticalFovTangent;
    mCamera.Position.z = -cameraZ;




    auto canvasPositionToRay = [&cameraZ](F x, F y) -> math::vector3<F> {
        //vector3<float>(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::vector3<F>(x, y, cameraZ);
    };

    const F QuaterPixelSize = F(0.5) * HalfPixelSize;
    math::vector3<F>* canvasDataPtr = mCanvas.GetBackbufferPtr();

    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            math::vector3<F>& pixel = canvasDataPtr[colIndex + rowOffset];


            const F pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const F pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

            Sample& sample = mImageSamples[colIndex + rowOffset];
            F x = F(2) * random<F>::value() - F(1);
            F y = F(2) * random<F>::value() - F(1);

            sample.pixelCol = colIndex;
            sample.pixelRow = rowIndex;

            sample.ray.set_origin(mCamera.Position);
            sample.ray.set_direction(canvasPositionToRay(pixelCenterX + x * HalfPixelSize, pixelCenterY + y * HalfPixelSize));
        }
    }
}

void LitRenderer::ResolveSamples()
{
    const math::vector3<F> StartReflectance = math::vector3<F>::one();
    TerminalRecord Record;
    math::vector3<F>* canvasDataPtr = mCanvas.GetBackbufferPtr();
    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            Sample& sample = mImageSamples[colIndex + rowOffset];
            math::vector3<F>& canvasPixel = canvasDataPtr[colIndex + rowOffset];
            canvasPixel += Trace(*mScene, sample.ray, Record);
        }
    }
    mCanvas.IncreaseSampleCount();
}

SimpleBackCamera::SimpleBackCamera(math::degree<F> verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value* F(0.5))
    , HalfVerticalFovTangent(math::tan(math::degree<F>(DegreeClampHelper(verticalFov).value* F(0.5))))
    , Position(0, 0, 0)
{ }


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

HitRecord Scene::DetectIntersecting(const math::ray3d<F>& ray, const SceneObject* excludeObject, F epsilon)
{
    HitRecord result;
    for (const SceneObject* obj : mSceneObjects)
    {
        if (excludeObject != obj)
        {
            HitRecord info = obj->IntersectWithRay(ray, epsilon);
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

void Scene::Create(F aspect)
{
    CreateScene(aspect, mSceneObjects);
    FindAllLights();
}

void Scene::FindAllLights()
{
    for (SceneObject* objectPtr : mSceneObjects)
    {
        PureLight_ForTest* lightMaterial = dynamic_cast<PureLight_ForTest*>(objectPtr->Material.get());
        if (lightMaterial != nullptr)
        {
            mSceneLights.push_back(objectPtr);
        }
    }
}

void Transform::UpdateWorldTransform()
{
    TransformMatrix = math::matrix4x4<F>::tr(Translate, Rotation);
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

HitRecord SceneSphere::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t0, t1;
    bool front = true;
    math::intersection result = math::intersect_sphere(ray, mWorldCenter, mSphere.radius_sqr(), error, t0, t1);
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

HitRecord SceneRect::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
    bool front = true;
    math::intersection result = math::intersect_rect(ray, mWorldPosition, mWorldNormal, mWorldTagent, Rect.extends(), mDualFace, error, t);
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

void SceneDisk::UpdateWorldTransform()
{
    SceneObject::UpdateWorldTransform();

    mWorldPosition = transform(Transform.TransformMatrix, Disk.position());
    mWorldNormal = transform(Transform.TransformMatrix, Disk.normal()); // no scale so there...
}

HitRecord SceneDisk::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t;
    bool front = true;
    math::intersection result = math::intersect_disk(ray, mWorldPosition, mWorldNormal, Disk.radius(), mDualFace, error, t);
    if (result == math::intersection::none)
    {
        return HitRecord();
    }
    else
    {
        front = math::dot(mWorldNormal, ray.direction()) < 0;
        return HitRecord(const_cast<SceneDisk*>(this), front, front ? mWorldNormal : -mWorldNormal, t);
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

HitRecord SceneCube::IntersectWithRay(const math::ray3d<F>& ray, F error) const
{
    F t0, t1;
    bool front = true;

    math::vector3<F> n0, n1;
    math::intersection result = math::intersect_cube(ray, mWorldPosition,
        mWorldAxisX, mWorldAxisY, mWorldAxisZ,
        Cube.width(), Cube.height(), Cube.depth(),
        error, t0, t1, n0, n1);
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


void Scene::CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects)
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

    SceneSphere* lSphere = new SceneSphere(); OutSceneObjects.push_back(lSphere);
    lSphere->SetRadius(SmallObjectSize);
    lSphere->SetTranslate(
        SceneCenterX - 20,
        SceneBottom + SmallObjectSize,
        SceneCenterZ - 5);
    lSphere->Material = std::make_unique<Lambertian>();

    SceneSphere* metalSphere = new SceneSphere(); OutSceneObjects.push_back(metalSphere);
    metalSphere->SetRadius(16);
    metalSphere->SetTranslate(
        SceneCenterX,
        SceneCenterY,
        SceneCenterZ + 10);
    metalSphere->Material = std::make_unique<Metal>(F(0.08));

    SceneSphere* dielectricSphereFloat = new SceneSphere(); OutSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(15);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 40,
        SceneCenterY,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material = std::make_unique<Dielectric>(F(1.5));

    SceneSphere* dielectricSphere = new SceneSphere(); OutSceneObjects.push_back(dielectricSphere);
    dielectricSphere->SetRadius(20);
    dielectricSphere->SetTranslate(
        SceneLeft + 30,
        SceneBottom + 20,
        SceneFar - 30);
    dielectricSphere->Material = std::make_unique<Dielectric>(F(1.4));

    SceneCube* lCube = new SceneCube(); OutSceneObjects.push_back(lCube);
    lCube->SetExtends(SmallObjectSize, BigObjectSize, SmallObjectSize);
    lCube->SetTranslate(
        SceneCenterX + 20 + BigObjectSize,
        SceneBottom + BigObjectSize,
        SceneCenterZ + 30);
    lCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(-30)));
    lCube->Material = std::make_unique<Lambertian>();

    SceneCube* rCube = new SceneCube(); OutSceneObjects.push_back(rCube);
    rCube->SetExtends(SmallObjectSize, SmallObjectSize, SmallObjectSize);
    rCube->SetTranslate(
        SceneCenterX + 15 + SmallObjectSize,
        SceneBottom + SmallObjectSize,
        SceneCenterZ + 5);
    rCube->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(60)));
    rCube->Material = std::make_unique<Lambertian>();

    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));

    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
    wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2));

    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
    wallFar->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

    SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
    LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
    LightDisk->SetExtends(25, 25);
    LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    F Intensity = 1;
    LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
}



void SimpleScene::CreateScene(F aspect, std::vector<SceneObject*>& OutSceneObjects)
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

    SceneSphere* mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
    mainSphere->SetRadius(20);
    mainSphere->SetTranslate(
        SceneCenterX + 20,
        SceneBottom + 22,
        SceneCenterZ + 10);
    mainSphere->Material = std::make_unique<Glossy>(1, 0.85, 0.5, 1.7);

    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
    wallLeft->Material = std::make_unique<Lambertian>(F(0.75), F(0.2), F(0.2));

    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
    wallRight->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(180)));
    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
    wallRight->Material = std::make_unique<Lambertian>(F(0.2), F(0.2), F(0.75));

    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
    wallTop->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    wallTop->Material = std::make_unique<Lambertian>(F(0.75), F(0.75), F(0.75));

    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
    wallBottom->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(90)));
    wallBottom->Material = std::make_unique<Lambertian>(F(0.2), F(0.75), F(0.2));

    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
    wallFar->SetExtends(SceneExtendX, SceneExtendY);
    wallFar->SetRotation(math::make_rotation_y_axis<F>(math::degree<F>(90)));
    wallFar->Material = std::make_unique<Lambertian>(F(0.6), F(0.6), F(0.6));

    SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
    LightDisk->SetDualFace(true);
    LightDisk->SetTranslate(SceneCenterX, SceneTop - F(0.01), SceneCenterZ + F(10));
    LightDisk->SetExtends(25, 25);
    LightDisk->SetRotation(math::make_rotation_z_axis<F>(math::degree<F>(-90)));
    F Intensity = 2.5;
    LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
}
