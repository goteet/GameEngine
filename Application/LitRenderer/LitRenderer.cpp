#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"


#include <random>
template<typename T>
struct random
{
    random() : generator(randomDevice()), distribution(F(0), F(1)) { }
    T operator()() { return _value(); }
    T operator()(T range_value) { return _range(range_value); }
    static random<T>& instance() { static random<T> instance; return instance; }
    static T value() { return instance()(); }
    static T range(T range_value) { return instance()(range_value); }

private:
    T _value() { return distribution(generator); }
    T _range(T range_value) { return _value() * T(2) * range_value - range_value; }

    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<F> distribution;
};

math::vector3<F> GenerateUnitSphereVector()
{
    static random<F> rand;
    while (true)
    {
        F x = rand(F(1));
        F y = rand(F(1));
        F z = rand(F(1));
        math::vector3<F> v = math::vector3<F>(x, y, z);
        if (math::magnitude_sqr(v) < F(1))
        {
            return v;
        }
    }
}


struct UVW
{
    math::vector3<F> u, v, w;

    UVW(const math::vector3<F>& normal) { from_normal(normal); }

    void from_normal(const math::vector3<F>& normal)
    {
        w = normalized(normal);
        v = fabs(w.x) > F(0.95) ? math::normalized_vector3<F>::unit_y() : math::normalized_vector3<F>::unit_x();
        v = normalized(math::cross(w, v));
        u = normalized(math::cross(w, v));
    }

    math::vector3<F> local(F x, F y, F z)
    {
        return normalized(x * u + y * v + z * w);
    }
};

math::normalized_vector3<F> GenerateHemisphereDirection(const math::vector3<F>& normal)
{
    static random<F> rand_theta;
    static random<F> rand_phi;
    F cosTheta = F(1) - F(2) * rand_theta();
    F sinTheta = sqrt(F(1) - cosTheta * cosTheta);
    math::radian<F> phi(math::TWO_PI<F> * rand_phi());
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    UVW uvw(normal);
    return uvw.local(x, y, z);
}

math::normalized_vector3<F> GenUniformHemisphereDirection(const math::vector3<F>& normal)
{
    static random<F> rand_theta;
    static random<F> rand_phi;
    // pdf = 1/2PI
    // => cdf = 1/2PI*phi*(1-cos_theta)
    // => f_phi = 1/2PI*phi       --> phi(x) = 2*PI*x
    // => f_theta = 1-cos_theta   --> cos_theta(x) = 1-x = x'
    F cosTheta = rand_theta(); //replace 1-e to e'
    F sinTheta = sqrt(F(1) - cosTheta * cosTheta);
    math::radian<F> phi(math::TWO_PI<F> * rand_phi());
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    return UVW(normal).local(x, y, z);
}


math::vector3<F> GenerateCosineWeightedHemisphereDirection(const math::vector3<F>& normal)
{
    static random<F> rand_theta;
    static random<F> rand_phi;
    // pdf = cos(theta) / Pi.
    F cosTheta_sqr = rand_theta(); //replace 1-e to e'
    F cosTheta = sqrt(cosTheta_sqr);
    F sinTheta = sqrt(F(1) - cosTheta_sqr);
    math::radian<F> phi(F(2) * math::PI<F> * rand_phi());
    F cosPhi = math::cos(phi);
    F sinPhi = math::sin(phi);

    F x = sinTheta * cosPhi;
    F y = sinTheta * sinPhi;
    F z = cosTheta;

    UVW uvw(normal);
    return uvw.local(x, y, z);
}

bool Lambertian::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering) const
{
    math::normalized_vector3<F> Direction = GenUniformHemisphereDirection(N);
    outScattering.set_origin(P);
    outScattering.set_direction(Direction);
    return true;
}

F Lambertian::ScatteringPDF(const math::vector3<F>& N, const math::vector3<F>& Scattering) const
{
    F cosTheta = math::dot(N, Scattering);
    return cosTheta < F(0) ? F(0) : F(1) / math::constant_value<F>::two_pi;
}

math::vector3<F> Reflect(const math::vector3<F>& In, const math::vector3<F>& N)
{
    return In - F(2) * math::dot(In, N) * N;
}

bool Metal::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering) const
{
    math::vector3<F> FuzzyDirection = Fuzzy * GenerateUnitSphereVector();
    math::vector3<F> Direction = Reflect(Ray.direction(), N) + FuzzyDirection;
    outScattering.set_origin(P);
    outScattering.set_direction(Direction);
    return math::dot(Direction, N) > F(0);
}

F Power5(F Base)
{
    F Ret = Base * Base;
    return Ret * Ret * Base;
}

F ReflectanceSchlick(F CosTheta, F eta1, F eta2)
{
    // Use Schlick's approximation for reflectance.
    F F0 = (eta1 - eta2) / (eta1 + eta2);
    F R0 = F0 * F0;
    F Base = F(1) - CosTheta;
    return R0 + (F(1) - R0) * Power5(Base);
}

bool Dielectric::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering) const
{
    const F AirRefractiveIndex = F(1.0003);

    const math::vector3<F>& InDirection = Ray.direction();

    F eta1 = IsFrontFace ? AirRefractiveIndex : RefractiveIndex;
    F eta2 = IsFrontFace ? RefractiveIndex : AirRefractiveIndex;
    const F RefractionRatio = eta1 / eta2;

    const F IdotN = -math::dot(InDirection, N);
    F CosTheta = math::clamp(IdotN);
    F SinThetaSqr = math::clamp(F(1) - CosTheta * CosTheta);
    F Det = F(1) - SinThetaSqr * RefractionRatio * RefractionRatio;
    bool RefractRay = Det >= F(0);

    bool ReflectRay = ReflectanceSchlick(CosTheta, eta1, eta2) > random<F>::value();
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
    return true;
}

bool PureLight_ForTest::Scattering(const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
    math::ray3d<F>& outScattering) const
{
    return false;
}

math::vector3<F> PureLight_ForTest::Emitting() const
{
    return Emission;
}


struct TerminalRecord
{
    static const int MaxRecursiveDepth = 100;

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

    F pdf = F(1);
    math::vector3<F> Li;
    math::ray3d<F> scattering;
    scattering.set_origin(shadingPoint);

    F N = F(1) / F(scene.GetLightCount() + 1);
    F Ln = scene.GetLightCount() * N;
    F e = random<F>::value();
    if (e < Ln)
    {
        int lightIndex = math::clamp(math::floor2<int>(scene.GetLightCount() * e), 0, scene.GetLightCount() - 1);
        SceneObject* LightObject = scene.GetLightSourceByIndex(lightIndex);
        if (LightObject != contactRecord.Object)
        {
            math::vector3<F> LightNormal;
            math::point3d<F> LightSamplePos = LightObject->SampleRandomPoint(LightNormal, pdf);
            math::vector3<F> lightDirectoin = LightSamplePos - shadingPoint;
            scattering.set_direction(lightDirectoin);
            F cosThetaPrime = math::dot(-LightNormal, scattering.direction());
            if (cosThetaPrime < -math::SMALL_NUM<F> && LightObject->IsDualface())
            {
                cosThetaPrime = -cosThetaPrime;
            }
            if (cosThetaPrime > math::SMALL_NUM<F>)
            {
                HitRecord lightHitRecord = scene.DetectIntersecting(scattering, nullptr, math::SMALL_NUM<F>);
                if (lightHitRecord.Object == LightObject)
                {
                    //direct light sampling.
                    F distSquare = math::dot(lightDirectoin, lightDirectoin);
                    F invDistSquare = (distSquare > F(0)) ? math::clamp(F(1) / distSquare) : F(1);
                    Li = LightObject->Material->Emitting() * cosThetaPrime * invDistSquare;
                }
            }
        }
    }
    else
    {
        if (material.Scattering(shadingPoint, normal, ray, contactRecord.IsFrontFace, scattering))
        {
            pdf = material.ScatteringPDF(normal, scattering.direction());
            F cosTheta = math::dot(normal, scattering.direction());
            math::vector3<F> reflectance = material.BRDF() * cosTheta / pdf;
            Li = Trace(scene, scattering, TerminalRecord.Next(reflectance));
        }
    }

    F cosTheta = math::dot(normal, scattering.direction());
    math::vector3<F> reflection = material.BRDF() * Li * cosTheta / pdf;
    math::vector3<F> emission = material.Emitting();
    return emission + reflection;
}

math::point3d<F> SceneRect::SampleRandomPoint(math::vector3<F>& outN, F& outPDF)
{
    outN = mWorldNormal;
    outPDF = F(1) / (Rect.width() * Rect.height());

    F e1 = random<F>::value() * Rect.extends().x;
    F e2 = random<F>::value() * Rect.extends().y;

    math::vector3<F> Bitangent = math::cross(mWorldNormal, mWorldTagent);
    return math::point3d<F>(mWorldPosition + e1 * mWorldTagent + e2 * Bitangent);
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
    metalSphere->Material = std::make_unique<Metal>(F(1.0), F(0.85), F(0.45), F(0.08));

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

    SceneSphere* dielectricSphereFloat = new SceneSphere(); OutSceneObjects.push_back(dielectricSphereFloat);
    dielectricSphereFloat->SetRadius(20);
    dielectricSphereFloat->SetTranslate(
        SceneCenterX + 20,
        SceneBottom + 22,
        SceneCenterZ + 10);
    dielectricSphereFloat->Material = std::make_unique<Lambertian>();

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
    F Intensity = 5;
    LightDisk->Material = std::make_unique<PureLight_ForTest>(Intensity, Intensity, Intensity);
}
