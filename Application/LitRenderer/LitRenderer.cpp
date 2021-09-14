#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Math/PredefinedConstantValues.h>
#include "LitRenderer.h"

template<typename value_type>
value_type LinearToGammaCorrected22(value_type value)
{
    const value_type DisplayGamma = value_type(2.2);
    const value_type InvDisplayGamma = value_type(1.0) / DisplayGamma;
    const value_type Inv2_4 = value_type(1.0) / value_type(2.4);

    if (value <= value_type(0.00031308))
    {
        return value_type(12.92) * value;
    }
    else if (value <= value_type(1.0))
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
    , mCanvasWidth(width)
    , mCanvasHeight(height)
    , mCanvasLinePitch(linePitch)
{
    mBackbuffer = new math::float3[mCanvasWidth * mCanvasHeight];

    for (int rowIndex = 0; rowIndex < mCanvasHeight; rowIndex++)
    {
        for (int colIndex = 0; colIndex < mCanvasWidth; colIndex++)
        {
            int pixelIndex = colIndex + rowIndex * mCanvasWidth;
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
    if (mNeedFlushBackbuffer)
    {
        FlushLinearColorToGammaCorrectedCanvasData();
        if (!mNeedFlushBackbuffer)
        {
            mNeedWindowUpdate = true;
        }
    }
    return mNeedWindowUpdate;
}

void RenderCanvas::FlushLinearColorToGammaCorrectedCanvasData()
{
    for (int rowIndex = 0; rowIndex < mCanvasHeight; rowIndex++)
    {
        for (int colIndex = 0; colIndex < mCanvasWidth; colIndex++)
        {
            int bufferIndex = colIndex + rowIndex * mCanvasWidth;
            int canvasIndex = colIndex * 3 + rowIndex * mCanvasLinePitch;

            const math::float3 color = mBackbuffer[bufferIndex];
            for (int compIndex = 0; compIndex < 3; compIndex++)
            {
                float c = LinearToGammaCorrected22(color.v[0]);
                mOutCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(c * 256.0f - math::EPSILON<float>);
            }
        }
    }
    mNeedFlushBackbuffer = false;
}

