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
    mBackbuffer = new Spectrum[count];
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
            mBackbuffer[pixelIndex].set(Float(0.0), Float(0.0), Float(0.0));
        }
    }
}

void LDRFilm::FlushTo(unsigned char* outCanvasDataPtr, int linePitch)
{
    Float invSampleCout = Float(1) / mSampleCount;
    for (int rowIndex = 0; rowIndex < CanvasHeight; rowIndex++)
    {
        int bufferRowOffset = rowIndex * CanvasWidth;
        int canvasRowOffset = rowIndex * linePitch;
        for (int colIndex = 0; colIndex < CanvasWidth; colIndex++)
        {
            int bufferIndex = colIndex + bufferRowOffset;
            int canvasIndex = colIndex * 3 + canvasRowOffset;

            const Spectrum& buffer = mBackbuffer[bufferIndex];
            for (int compIndex = 2; compIndex >= 0; compIndex--)
            {
                Float c = LinearToGamma22Corrected(buffer[compIndex] * invSampleCout);
                outCanvasDataPtr[canvasIndex++] = math::floor2<unsigned char>(math::saturate(c) * Float(256.0) - Float(0.0001));
            }
        }
    }
}
