#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"
#include "Integrator.h"


static const int BlockSize = 64;
namespace
{
    const bool DEBUG = false;
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
                const Float IoR = Float(1.1);
                Float Ks = Float(0.5);
                if (DEBUG)
                {
                    mainSphere->Material = MakeMicrofacetGGXMaterialDebug(roughness, IoR, Ks);
                }
                else
                {
                    Float Kd = Float(1) - Ks;
                    mainSphere->Material = MakePlasticMaterial(Kd, Spectrum(Float(0.5)), Ks, roughness, IoR);
                }
            }


            if (!DEBUG)
            {
                Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
                Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
                Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
                Spectrum Gray(Float(0.75));
                Spectrum DarkGray(Float(0.6));

                SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
                wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
                wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
                wallLeft->Material = MakeMatteMaterial(Red);

                SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
                wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
                wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
                wallRight->SetExtends(SceneExtendZ, SceneExtendY);
                wallRight->Material = MakeMatteMaterial(Blue);

                SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
                wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
                wallTop->SetExtends(SceneExtendZ, SceneExtendX);
                wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
                wallTop->Material = MakeMatteMaterial(Gray);

                SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
                wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
                wallFar->SetExtends(SceneExtendX, SceneExtendY);
                wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
                wallFar->Material = MakeMatteMaterial(DarkGray);

                SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
                wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
                wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
                wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
                wallBottom->Material = MakeMatteMaterial(Green);
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
            LambertianShere->Material = MakeMatteMaterial();


            SceneSphere* orenNayerSphere = new SceneSphere(); OutSceneObjects.push_back(orenNayerSphere);
            orenNayerSphere->SetRadius(20);
            orenNayerSphere->SetTranslate(
                SceneCenterX - 30,
                SceneBottom + 22,
                SceneCenterZ + 10);
            orenNayerSphere->Material = MakeMatteMaterial(Spectrum::one(), 0.25_radd);

            Spectrum Red(Float(0.75), Float(0.2), Float(0.2));
            Spectrum Green(Float(0.2), Float(0.75), Float(0.2));
            Spectrum Blue(Float(0.2), Float(0.2), Float(0.75));
            Spectrum Gray(Float(0.75));
            Spectrum DarkGray(Float(0.6));

            SceneRect* wallLeft = new SceneRect(); OutSceneObjects.push_back(wallLeft);
            wallLeft->SetTranslate(SceneLeft, SceneCenterY, SceneCenterZ);
            wallLeft->SetExtends(SceneExtendZ, SceneExtendY);
            wallLeft->Material = MakeMatteMaterial(DarkGray);

            SceneRect* wallRight = new SceneRect(); OutSceneObjects.push_back(wallRight);
            wallRight->SetTranslate(SceneRight, SceneCenterY, SceneCenterZ);
            wallRight->SetRotation(math::make_rotation_y_axis<Float>(180_degd));
            wallRight->SetExtends(SceneExtendZ, SceneExtendY);
            wallRight->Material = MakeMatteMaterial(DarkGray);

            SceneRect* wallTop = new SceneRect(); OutSceneObjects.push_back(wallTop);
            wallTop->SetTranslate(SceneCenterX, SceneTop, SceneCenterZ);
            wallTop->SetExtends(SceneExtendZ, SceneExtendX);
            wallTop->SetRotation(math::make_rotation_z_axis<Float>(-90_degd));
            wallTop->Material = MakeMatteMaterial(Gray);

            SceneRect* wallBottom = new SceneRect(); OutSceneObjects.push_back(wallBottom);
            wallBottom->SetTranslate(SceneCenterX, SceneBottom, SceneCenterZ);
            wallBottom->SetExtends(SceneExtendZ, SceneExtendX);
            wallBottom->SetRotation(math::make_rotation_z_axis<Float>(90_degd));
            wallBottom->Material = MakeMatteMaterial(Blue);

            SceneRect* wallFar = new SceneRect(); OutSceneObjects.push_back(wallFar);
            wallFar->SetTranslate(SceneCenterX, SceneCenterY, SceneFar);
            wallFar->SetExtends(SceneExtendX, SceneExtendY);
            wallFar->SetRotation(math::make_rotation_y_axis<Float>(90_degd));
            wallFar->Material = MakeMatteMaterial(Red);

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
    , mCamera(50_degd)
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
    const Float PixelSize = Float(1);
    const Float HalfPixelSize = PixelSize * Float(0.5);
    Float HalfWidth = mFilm.CanvasWidth * Float(0.5) * PixelSize;
    Float HalfHeight = mFilm.CanvasHeight * Float(0.5) * PixelSize;
    //     <---> (half height)
    //  .  o----. (o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  .  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    Float CameraZ = HalfHeight / mCamera.HalfVerticalFovTangent;
    auto CanvasPositionToRay = [this, CameraZ](Float x, Float y) -> math::vector3<Float>
    {
        //vector3<float>(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::vector3<Float>(x * mCamera.Left + y * mCamera.Up + CameraZ * mCamera.Forward + mCamera.Position);
    };


    const int NumVerticalBlock = mFilm.CanvasHeight / BlockSize + 1;
    const int NumHorizontalBlock = mFilm.CanvasWidth / BlockSize + 1;
    std::vector<Task> GenerateSampleTasks;
    for (int BlockIndexV = 0; BlockIndexV < NumVerticalBlock; BlockIndexV += 1)
    {
        for (int BlockIndexH = 0; BlockIndexH < NumHorizontalBlock; BlockIndexH += 1)
        {
            Task GenerateSampleTask = Task::Start(ThreadName::Worker,
                [this, PixelSize, HalfPixelSize, HalfHeight, HalfWidth, BlockIndexV, BlockIndexH, CanvasPositionToRay](::Task&)
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
                            const Float pixelCenterX = ColIndex * PixelSize + HalfPixelSize - HalfWidth;
                            const Float pixelCenterY = RowIndex * PixelSize + HalfPixelSize - HalfHeight;

                            Sample& Samples = mCameraRaySamples[ColIndex + RowOffset];
                            Samples.PixelRow = RowIndex;
                            Samples.PixelCol = ColIndex;
                            Samples.Ray.set_origin(mCamera.Position);
                            Samples.Ray.set_direction(CanvasPositionToRay(pixelCenterX, pixelCenterY));

                            Samples.RecordP1 = mScene->DetectIntersecting(Samples.Ray, nullptr, math::SMALL_NUM<Float>);
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
    mCamera.Left = Direction::unit_x();
    mCameraDirty = true;
}

void LitRenderer::MoveCamera(const math::vector3<Float>& Offset)
{
    mCamera.Position += Offset.x * mCamera.Left + Offset.z * mCamera.Forward;
    mCameraDirty = true;
}

void LitRenderer::RotateCamera(const Radian& Yaw, const Radian& Pitch)
{
    math::quaterniond RotationYaw = math::quaterniond(mCamera.Up, Yaw);

    mCamera.Forward = math::rotate(RotationYaw, mCamera.Forward);
    mCamera.Left = math::cross(mCamera.Up, mCamera.Forward);

    math::quaterniond RotationPitch = math::quaterniond(mCamera.Left, Pitch);
    mCamera.Forward = math::rotate(RotationPitch, mCamera.Forward);
    mCamera.Up = math::cross(mCamera.Forward, mCamera.Left);
    mCameraDirty = true;
}

void LitRenderer::ResolveSamples()
{
    const Sample* Samples = mCameraRaySamples;
    AccumulatedSpectrum* AccumulatedBufferPtr = mFilm.GetBackbufferPtr();

    std::vector<Task> PixelIntegrationTasks;

    if (MaxSampleCount > 0 && (Frame / 4) >= MaxSampleCount)
    {
        return;
    }

    const int RenderBlockSize = 2;
    const int NumVerticalBlock = mFilm.CanvasHeight / RenderBlockSize + 1;
    const int NumHorizontalBlock = mFilm.CanvasWidth / RenderBlockSize + 1;

    for (int BlockIndexV = (Frame++ + 1) / 2 % 2; BlockIndexV < NumVerticalBlock; BlockIndexV += 2)
    {
        for (int BlockIndexH = Frame % 2; BlockIndexH < NumHorizontalBlock; BlockIndexH += 2)
        {
            Task EvaluateLiTask = Task::Start(ThreadName::Worker,
                [this, BlockSize = RenderBlockSize, MaxSampleCount = MaxSampleCount, BlockIndexV, BlockIndexH, AccumulatedBufferPtr, Samples](::Task&)
                {
                    PathIntegrator pathIntegrator;
                    DebugIntegrator debugIntegrator;
                    Integrator& IntegratorRef = DEBUG ? (Integrator&)debugIntegrator : (Integrator&)pathIntegrator;

                    int RowStart = BlockIndexV * BlockSize;
                    int RowEnd = math::min2(RowStart + BlockSize, mFilm.CanvasHeight);
                    int ColStart = BlockIndexH * BlockSize;
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
                                CanvasPixel.Value += IntegratorRef.EvaluateLi(*mScene, Sample.Ray, Sample.RecordP1);
                                CanvasPixel.Count += 1;
                            }
                        }
                    }
                });
            PixelIntegrationTasks.push_back(EvaluateLiTask);
        }
    }

    ResolveSampleTask = Task::WhenAll(ThreadName::Worker, [&](Task& Task)
        {
            mFilm.FlushTo(mSystemCanvasDataPtr, mCanvasLinePitch, Task);
        }
    , PixelIntegrationTasks);
}

SimpleBackCamera::SimpleBackCamera(Degree verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value* Float(0.5))
    , HalfVerticalFovTangent(math::tan(Degree(DegreeClampHelper(verticalFov).value* Float(0.5))))
    , Position(0, 0, 0)
{ }
