#include <cassert>
#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"

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
    mBackbuffer = new math::float3[CanvasWidth * CanvasHeight];

    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int pixelIndex = colIndex + rowIndex * CanvasWidth;
            mBackbuffer[pixelIndex].set(0.5, 0.5, 0.5);
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

            const math::float3& color = mBackbuffer[bufferIndex];
            for (int compIndex = 2; compIndex >= 0; compIndex--)
            {
                float c = LinearToGammaCorrected22(color.v[compIndex]);
                assert(canvasIndex < (rowIndex + 1) * CanvasLinePitch);
                mOutCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(c * 256.0f - math::EPSILON<float>);
            }
        }
    }
    NeedFlushBackbuffer = false;
}

LitRenderer::LitRenderer(unsigned char * canvasDataPtr, int canvasWidth, int canvasHeight, int canvasLinePitch)
    : mCanvas(canvasDataPtr, canvasWidth, canvasHeight, canvasLinePitch)
    , mCamera(math::degree<float>(60))
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
    const float PixelSize = 0.5f;
    const float HalfPixelSize = PixelSize * 0.5f;
    float halfWidth = mCanvas.CanvasWidth * 0.5f * PixelSize;
    float halfHeight = mCanvas.CanvasHeight * 0.5f * PixelSize;
    //     <---> (half height)
    //  +  o----.¡¡(o=origin)
    //  |  |  /   Asumed canvas is at origin(0,0,0),
    //  |  | /    and camera is placed at neg-z-axis,
    //  +  |/
    // (z) .      tan(half_fov) = halfHeight / cameraZ.
    float cameraZ = halfHeight / mCamera.HalfVerticalFovTangent;
    mCamera.Position.set(0, 0, -cameraZ);

    Sample sample;
    sample.ray.set_origin(mCamera.Position);


    auto canvasPositionToRay = [&cameraZ](float x, float y) -> math::float3 {
        //float3(x,y,0) - camera.position;
        //  x = x - 0;
        //  y = y - 0;
        //  z = 0 - camera.position.z
        return math::float3(x, y, cameraZ);
    };

    const float QuaterPixelSize = 0.5f * HalfPixelSize;
    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            std::vector<Sample>& samples = mImageSamples[colIndex + rowOffset];

            sample.pixelCol = colIndex;
            sample.pixelRow = rowIndex;

            const float pixelCenterX = colIndex * PixelSize + HalfPixelSize - halfWidth;
            const float pixelCenterY = rowIndex * PixelSize + HalfPixelSize - halfHeight;

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
    math::float3* canvasDataPtr = mCanvas.GetBackbufferPtr();
    for (int rowIndex = 0; rowIndex < mCanvas.CanvasHeight; rowIndex++)
    {
        int rowOffset = rowIndex * mCanvas.CanvasWidth;
        for (int colIndex = 0; colIndex < mCanvas.CanvasWidth; colIndex++)
        {
            std::vector<Sample>& samples = mImageSamples[colIndex + rowOffset];
            math::float3& canvasPixel = canvasDataPtr[colIndex + rowOffset];

            const size_t sampleCount = samples.size();
            math::float3 direction;
            for (const Sample& sample : samples)
            {
                direction += sample.ray.direction();
            }
            direction *= (1.0f / sampleCount);
            auto c = direction * 0.5f + 0.5f;
            canvasPixel.set(c.x, c.y, 0.0f);
        }
    }
}

SimpleBackCamera::SimpleBackCamera(math::degree<float> verticalFov)
    : HalfVerticalFov(DegreeClampHelper(verticalFov).value * 0.5f)
    , HalfVerticalFovTangent(::tan(DegreeClampHelper(verticalFov).value * 0.5f))
    , Position(0, 0, 0)
{ }