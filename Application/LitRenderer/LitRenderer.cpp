#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"
#include "Integrator.h"


static const int BlockSize = 64;
namespace
{
    const bool DEBUG = false;
    const bool DEBUGScene = DEBUG || true;
    class SimpleScene : public Scene
    {
        virtual void CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects) override
        {
            const Float SceneSize = 60;
            const Float SceneNear = -30;
            const Float SceneFar = 30;
            const Float SceneBottom = -SceneSize;
            const Float SceneTop = SceneSize;
            const Float SceneLeft = -SceneSize * aspect;
            const Float SceneRight = SceneSize * aspect;

            const Float SceneCenterX = (SceneLeft + SceneRight) * Float(0.5);
            const Float SceneCenterY = (SceneBottom + SceneTop) * Float(0.5);
            const Float SceneCenterZ = (SceneNear + SceneFar) * Float(0.5);
            const Float SceneExtendX = (SceneRight - SceneLeft) * Float(0.5);
            const Float SceneExtendY = (SceneTop - SceneBottom) * Float(0.5);
            const Float SceneExtendZ = (SceneFar - SceneNear) * Float(0.5);

            const Float SceneDistance = 5;
            const Float SmallObjectSize = 8;
            const Float BigObjectSize = SmallObjectSize * Float(1.75);

            if (DEBUGScene)
            {
                SceneSphere* mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
                mainSphere->SetRadius(20);
                mainSphere->SetTranslate(SceneCenterX, SceneCenterY, SceneCenterZ - Float(10));
                mainSphere->SetTranslate(SceneCenterX - Float(15), SceneCenterY - Float(20), SceneCenterZ - Float(10));
                mainSphere->Material = Material::CreateMicrofacetGGX_Debug(Float(0.01), SpecularColor::Gold());

                mainSphere->Material->Type = Material::Transparency;


                mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
                mainSphere->SetRadius(10);
                //mainSphere->SetTranslate(SceneCenterX, SceneCenterY, SceneCenterZ - Float(10));
                mainSphere->SetTranslate(SceneCenterX + Float(10), SceneCenterY - Float(30), SceneCenterZ + Float(20));
                mainSphere->Material = Material::CreateMicrofacetGGX_Debug(Float(0.01), SpecularColor::Gold());
                
                mainSphere->Material->Type = Material::Opaque;
            }
            else
            {
                for (int i = 0; i < 16; i++)
                {
                    SceneSphere* mainSphere = new SceneSphere(); OutSceneObjects.push_back(mainSphere);
                    mainSphere->SetRadius(10);

                    Float OffsetX = Float(-40 + (i % 4) * 25);
                    Float OffsetY = Float((i / 4) * 25);
                    Float OffsetZ = 0;
                    mainSphere->SetTranslate(
                        SceneCenterX + OffsetX,
                        SceneBottom + 20 + OffsetY,
                        SceneCenterZ + OffsetZ);

                    const Float roughness = Float(0.1 + 0.05 * i);
                    const Float IoR = Float(1.8 - i * 0.05);

                    mainSphere->Material = Material::CreatePlastic(Spectrum(Float(0.5)), roughness, Spectrum(1));
                    mainSphere->Material = Material::CreateMicrofacetGGX_Debug(roughness, SpecularColor::Gold());
                    //mainSphere->Material = Material::CreateAshikhminAndShirley(roughness, Spectrum(0.1), SpecularColor::Gold());
                    mainSphere->Material->Type = (i % 3) == 2 ? Material::Transparency : Material::Opaque;
                }
            }


            
            {
                Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
                Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
                Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
                Spectrum Gray(Float(0.75));
                Spectrum DarkGray(Float(0.6));

                if (!DEBUGScene)
                {
                    SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
                    wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
                    wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
                    wallLeft->Material = Material::CreateMatte(Red);

                    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
                    wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
                    wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
                    wallRight->SetExtends(SceneExtendZ, SceneExtendY);
                    wallRight->Material = Material::CreateMatte(Blue);

                    SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
                    wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
                    wallTop->SetExtends(SceneExtendZ, SceneExtendX);
                    wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
                    wallTop->Material = Material::CreateMatte(Gray);

                    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
                    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
                    wallFar->SetExtends(SceneExtendX, SceneExtendY);
                    wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
                    wallFar->Material = Material::CreateMatte(DarkGray);

                    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
                    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
                    wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
                    wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
                    wallBottom->Material = Material::CreateMatte(Green);
                }
                else
                {

                    SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
                    wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar + Float(150));
                    wallFar->SetExtends(SceneExtendX * 100, SceneExtendY * 100);
                    wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
                    wallFar->Material = Material::CreateMatte(Red);

                    SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
                    wallRight->SetTranslate(SceneRight + Float(10), SceneCenterY, SceneCenterZ);
                    wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
                    wallRight->SetExtends(SceneExtendZ * 100, SceneExtendY * 100);
                    wallRight->Material = Material::CreateMatte(Blue);

                    SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
                    wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ );
                    wallBottom->SetExtends(SceneExtendZ * 100, SceneExtendX * 100);
                    wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
                    wallBottom->Material = Material::CreateMatte(Green);
                }
            }

            {
                SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
                LightDisk->SetDualFace(true);
                LightDisk->SetTranslate(SceneCenterX, SceneTop - Float(0.01), SceneCenterZ - Float(10));
                //LightDisk->SetRadius(50);
                LightDisk->SetExtends(25, 25);
                LightDisk->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
                Float Intensity = 1.0;

                LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
            }

            if(false)
            {
                SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
                LightDisk->SetDualFace(true);
                LightDisk->SetTranslate(SceneCenterX, SceneCenterY, SceneNear - Float(200));
                LightDisk->SetExtends(SceneSize * 1.5, SceneSize);
                LightDisk->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
                Float Intensity = 2.5;
                LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
            }
        }
    };

    class OrenNayerComparisionScene : public Scene
    {
        virtual void CreateScene(Float aspect, std::vector<SceneObject*>& OutSceneObjects) override
        {
            const Float SceneSize = 60;
            const Float SceneNear = 1;
            const Float SceneFar = SceneSize * Float(2.5);
            const Float SceneBottom = -SceneSize;
            const Float SceneTop = SceneSize;
            const Float SceneLeft = -SceneSize * aspect;
            const Float SceneRight = SceneSize * aspect;

            const Float SceneCenterX = (SceneLeft + SceneRight) * Float(0.5);
            const Float SceneCenterY = (SceneBottom + SceneTop) * Float(0.5);
            const Float SceneCenterZ = (SceneNear + SceneFar) * Float(0.5);
            const Float SceneExtendX = (SceneRight - SceneLeft) * Float(0.5);
            const Float SceneExtendY = (SceneTop - SceneBottom) * Float(0.5);
            const Float SceneExtendZ = (SceneFar - SceneNear) * Float(0.5);

            const Float SceneDistance = 5;
            const Float SmallObjectSize = 8;
            const Float BigObjectSize = SmallObjectSize * Float(1.75);

            SceneSphere* LambertianShere = new SceneSphere(); OutSceneObjects.push_back(LambertianShere);
            LambertianShere->SetRadius(20);
            LambertianShere->SetTranslate(
                SceneCenterX + 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            LambertianShere->Material = Material::CreateMatte();


            SceneSphere* orenNayerSphere = new SceneSphere(); OutSceneObjects.push_back(orenNayerSphere);
            orenNayerSphere->SetRadius(20);
            orenNayerSphere->SetTranslate(
                SceneCenterX - 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            orenNayerSphere->Material = Material::CreateMatte(Spectrum::one(), 0.25_radd);

            Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
            Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
            Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
            Spectrum Gray(Float(0.75));
            Spectrum DarkGray(Float(0.6));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = Material::CreateMatte(DarkGray);

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = Material::CreateMatte(DarkGray);

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            wallTop->Material = Material::CreateMatte(Gray);

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
            wallBottom->Material = Material::CreateMatte(Blue);

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
            wallFar->Material = Material::CreateMatte(Red);

            SceneRect* LightDisk = new SceneRect(); OutSceneObjects.push_back(LightDisk);
            LightDisk->SetDualFace(true);
            LightDisk->SetTranslate(SceneCenterX, SceneTop - Float(0.01), SceneCenterZ + Float(10));
            LightDisk->SetExtends(25, 25);
            LightDisk->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            Float Intensity = 4.5;
            LightDisk->LightSource = std::make_unique<LightSource>(Intensity, Intensity, Intensity);
        }
    };

}

LitRenderer::LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvasLinePitch(canvasLinePitch)
    , mSystemCanvasDataPtr(canvasDataPtr)
    , mFilm(canvasWidth, canvasHeight)
    , mCamera(50_degd, canvasWidth, canvasHeight)
    , mScene(std::make_unique<SimpleScene>())
{
    mCamera.Position.set(0, 0, -130);
    mScene->Create(Float(canvasWidth) / Float(canvasHeight));
    mCameraRaySamples = new Sample[canvasWidth * canvasHeight];
}

LitRenderer::~LitRenderer()
{
    SafeDeleteArray(mCameraRaySamples);
}

void LitRenderer::InitialSceneTransforms()
{
    mScene->UpdateWorldTransform();
}

void LitRenderer::GenerateCameraRays()
{
    std::vector<Task> GenerateSampleTasks;
    const int NumBlockX = (mFilm.CanvasWidth + BlockSize - 1) / BlockSize;
    const int NumBlockY = (mFilm.CanvasHeight + BlockSize - 1) / BlockSize;
    for (int BlockIndexV = 0; BlockIndexV < NumBlockY; BlockIndexV += 1)
    {
        for (int BlockIndexH = 0; BlockIndexH < NumBlockX; BlockIndexH += 1)
        {
            Task GenerateSampleTask = Task::Start(ThreadName::Worker,
                [this, BlockIndexV, BlockIndexH, &Camera = mCamera](::Task&)
                {   
                    random<Float> RandomGeneratorPickingPixel;
                    int RowStart = BlockIndexV * BlockSize;
                    int RowEnd = math::min2(RowStart + BlockSize, mFilm.CanvasHeight);
                    int ColStart = BlockIndexH * BlockSize;
                    int ColEnd = math::min2(ColStart + BlockSize, mFilm.CanvasWidth);

                    for (int RowIndex = RowStart; RowIndex < RowEnd; RowIndex++)
                    {
                        int RowOffset = RowIndex * mFilm.CanvasWidth;
                        for (int ColIndex = ColStart; ColIndex < ColEnd; ColIndex++)
                        {
                            Sample& Sample = mCameraRaySamples[ColIndex + RowOffset];
                            Sample.PixelRow = RowIndex;
                            Sample.PixelCol = ColIndex;
                            Sample.Ray = Camera.GenerateCameraRay(ColIndex, RowIndex);
                            Sample.RecordP1 = mScene->DetectIntersecting(Sample.Ray, nullptr, math::SMALL_NUM<Float>);
                        }
                    }

                }
            );
            GenerateSampleTasks.push_back(GenerateSampleTask);
        }
    }

    for (auto& Task : GenerateSampleTasks)
    {
        Task.SpinWait();
    }
}

void LitRenderer::Initialize()
{
    InitialSceneTransforms();
    mCamera.PositionBak = mCamera.Position;
}

bool LitRenderer::GenerateImageProgressive()
{
    if (ResolveSampleTask.IsCompleted())
    {
        if (mCameraDirty)
        {
            Frame = 0;
            mFilm.Clear();
            GenerateCameraRays();
            mCameraDirty = false;
        }
        ResolveSamples();
        return true;
    }
    return false;
}

bool LitRenderer::NeedUpdate()
{
    return ResolveSampleTask.IsCompleted();
}

void LitRenderer::ResetCamera()
{
    mCamera.Position = mCamera.PositionBak;
    mCamera.Up = Direction::unit_y();
    mCamera.Forward = Direction::unit_z();
    mCamera.Right = Direction::unit_x();
    mCameraDirty = true;
}

void LitRenderer::MoveCamera(const math::vector3<Float>& Offset)
{
    mCamera.Position += Offset.x * mCamera.Right + Offset.y * Direction::unit_y() + Offset.z * mCamera.Forward;
    mCameraDirty = true;
}

void LitRenderer::RotateCamera(const Radian& Yaw, const Radian& Pitch)
{
    math::quaterniond RotationYaw = math::quaterniond(mCamera.Up, Yaw);

    mCamera.Forward = math::rotate(RotationYaw, mCamera.Forward);
    mCamera.Right = math::cross(mCamera.Up, mCamera.Forward);

    math::quaterniond RotationPitch = math::quaterniond(mCamera.Right, Pitch);
    mCamera.Forward = math::rotate(RotationPitch, mCamera.Forward);
    mCamera.Up = math::cross(mCamera.Forward, mCamera.Right);
    mCameraDirty = true;
}

template<typename Integrator>
struct IntegratorForwarder
{
    Integrator _Integrator;

    template<class... Args>
    Spectrum Evaluate(Args&&...args)
    {
        return _Integrator.EvaluateLi(std::forward<Args>(args)...);
    }
};
void LitRenderer::ResolveSamples()
{
    const Sample* Samples = mCameraRaySamples;
    AccumulatedSpectrum* AccumulatedBufferPtr = mFilm.GetBackbufferPtr();

    std::vector<Task> PixelIntegrationTasks;

    if (MaxSampleCount > 0 && (Frame / 4) >= MaxSampleCount)
    {
        return;
    }

    const int RenderBlockSize = 4;
    const int NumBlockX = (mFilm.CanvasWidth + RenderBlockSize - 1) / RenderBlockSize;
    const int NumBlockY = (mFilm.CanvasHeight + RenderBlockSize - 1) / RenderBlockSize;
    for (int BlockIndexY = (Frame++ + 1) / 2 % 2; BlockIndexY < NumBlockY; BlockIndexY += 2)
    {
        for (int BlockIndexX = Frame % 2; BlockIndexX < NumBlockX; BlockIndexX += 2)
        {
            Task EvaluateLiTask = Task::Start(ThreadName::Worker,
                [this, BlockSize = RenderBlockSize, MaxSampleCount = MaxSampleCount, BlockIndexY, BlockIndexX, AccumulatedBufferPtr, Samples](::Task&)
                {
                    IntegratorForwarder<DistributionIntegrator> IntegratorRef;// DEBUG ? (Integrator&)debugIntegrator : (Integrator&)pathIntegrator;

                    int RowStart = BlockIndexY * BlockSize;
                    int RowEnd = math::min2(RowStart + BlockSize, mFilm.CanvasHeight);
                    int ColStart = BlockIndexX * BlockSize;
                    int ColEnd = math::min2(ColStart + BlockSize, mFilm.CanvasWidth);
                    for (int RowIndex = RowStart; RowIndex < RowEnd; RowIndex++)
                    {
                        int RowOffset = RowIndex * mFilm.CanvasWidth;
                        for (int ColIndex = ColStart; ColIndex < ColEnd; ColIndex++)
                        {
                            const Sample& Sample = Samples[ColIndex + RowOffset];
                            AccumulatedSpectrum& CanvasPixel = AccumulatedBufferPtr[ColIndex + RowOffset];

                            const bool bGenerateMore = MaxSampleCount <= 0 || MaxSampleCount > (int)CanvasPixel.Count;
                            if (bGenerateMore)
                            {
                                //Float NdotL = math::dot(Sample.RecordP1.SurfaceNormal, -Sample.Ray.direction());
                                //CanvasPixel.Value += Spectrum(NdotL);
                                CanvasPixel.Value += IntegratorRef.Evaluate(*mScene, Sample.Ray, Sample.RecordP1);
                                CanvasPixel.Count += 1;

                                mFilm.FlushTo(CanvasPixel, RowIndex, ColIndex, this->mSystemCanvasDataPtr, mCanvasLinePitch);
                            }
                        }
                    }
                });
            PixelIntegrationTasks.push_back(EvaluateLiTask);
        }
    }

    ResolveSampleTask = Task::WhenAll(ThreadName::Worker, [](auto) {}, PixelIntegrationTasks);
}

template <
    uint32_t MIN_FOV_DEGREE = 30,
    uint32_t MAX_FOV_DEGREE = 160
> struct LimitedHalfFOV : Degree {
    LimitedHalfFOV(Degree degree)
        : Degree(Float(0.5) * math::clamp(degree.value
            , Float(MIN_FOV_DEGREE)
            , Float(MAX_FOV_DEGREE))
        ){ }
};

Float4x4 CalcSampleToCameraMatrix(uint32_t width, uint32_t height, Float tan)
{
    /*
    *      .
    *     /|
    *    / |  h
    *   /  | --- = tan(fov/2) =>  h = z*tan(fov/2) = tan(0.5fov);
    *  /   |  z
    * o----+-----> z=1
    *
    */
    Float aspect = Float(1) * width / height;
    return Float4x4::scale(tan * aspect, tan, Float(1.0))
        * Float4x4::translation(Float(-1.0), Float(-1.0), Float(1.0))
        * Float4x4::scale(Float(2.0) / width, Float(2.0) / height, Float(1.0))
        * Float4x4::translation(Float(0.5), Float(0.5), Float(0.0));
}

SimpleBackCamera::SimpleBackCamera(Degree verticalFOV, uint32_t filmWidth, uint32_t filmHeight)
    : HalfVerticalFov(LimitedHalfFOV<>(verticalFOV))
    , HalfVerticalFovTangent(math::tan(LimitedHalfFOV<>(verticalFOV)))
    , Position(0, 0, 0)
    , mFilmWidth(filmWidth), mFilmHeight(filmHeight)
    , mSampleToCamera(CalcSampleToCameraMatrix(filmWidth, filmHeight, math::tan(LimitedHalfFOV<>(verticalFOV))))
{

}

Ray SimpleBackCamera::GenerateCameraRay(uint32_t x, uint32_t y)
{
    if (x > mFilmWidth)  x = mFilmWidth;
    if (y > mFilmHeight) y = mFilmHeight;

    Point cameraP = math::transform(mSampleToCamera, Point(x, y, Float(0)));
    Direction d = cameraP.x * Right + cameraP.y * Up + cameraP.z * Forward;
    return Ray{ Position, d };
}
