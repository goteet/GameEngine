#include "LDRFilm.h"
#include <Foundation/Base/MemoryHelper.h>

template<typename value_type>
value_type LinearToGamma22Corrected(value_type value)
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


LDRFilm::LDRFilm(int width, int height)
    : CanvasWidth(width)
    , CanvasHeight(height)
{
    int count = CanvasWidth * CanvasHeight;
    mBackbuffer = new math::vector3<F>[count];
    Clear();
}

LDRFilm::~LDRFilm()
{
    SafeDeleteArray(mBackbuffer);
}

void LDRFilm::Clear()
{

    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int pixelIndex = colIndex + rowIndex * CanvasWidth;
            mBackbuffer[pixelIndex].set(F(0.0), F(0.0), F(0.0));
        }
    }
}

void LDRFilm::FlushTo(unsigned char* outCanvasDataPtr, int linePitch)
{
    F invSampleCout = F(1) / mSampleCount;
    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        int bufferRowOffset = rowIndex * CanvasWidth;
        int canvasRowOffset = rowIndex * linePitch;
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int bufferIndex = colIndex + bufferRowOffset;
            int canvasIndex = colIndex * 3 + canvasRowOffset;

            const math::vector3<F>& buffer = mBackbuffer[bufferIndex];
            for (int compIndex = 2; compIndex >= 0; compIndex--)
            {
                F c = LinearToGamma22Corrected(buffer[compIndex] * invSampleCout);
                outCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(math::saturate(c) * F(256.0) - F(0.0001));
            }
        }
    }
}
