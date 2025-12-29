#pragma once
#pragma once
#include <memory>
#include <vector>
#include "PreInclude.h"
#include "LDRFilm.h"
#include "Material.h"
#include "Scene.h"


class SimpleBackCamera
{
public:
    SimpleBackCamera(Degree verticalFOV);
    Direction Up = Direction::unit_y();
    Direction Forward = Direction::unit_z();
    Direction Right = Direction::unit_x();
    Point PositionBak;
    Point Position;
    const Radian HalfVerticalFov;
    const Float HalfVerticalFovTangent;
};

class LitRenderer
{
public:
    LitRenderer(unsigned char* canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch);
    ~LitRenderer();

    void Initialize();
    bool GenerateImageProgressive();
    bool NeedUpdate();
    void ResetCamera();
    void MoveCamera(const math::vector3<Float>& Offset);
    void RotateCamera(const Radian& Yaw, const Radian& Pitch);

private:
    void InitialSceneTransforms();
    void GenerateCameraRays();
    void ResolveSamples();

    static const int MaxLightRaySampleCount = -1;
    static const int MaxSampleCount = MaxLightRaySampleCount;

    struct Sample
    {
        Ray Ray;
        SurfaceIntersection RecordP1;
        int PixelRow = 0, PixelCol = 0;
    };

    const int mCanvasLinePitch;
    unsigned char* mSystemCanvasDataPtr;
    LDRFilm mFilm;
    SimpleBackCamera mCamera;
    std::unique_ptr<Scene> mScene;
    Sample* mCameraRaySamples;
    int Frame = 0;
    bool mCameraDirty = true;
    Task ResolveSampleTask;
};
