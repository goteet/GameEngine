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
    mBackbuffer = new AccumulatedSpectrum[count];
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
            mBackbuffer[pixelIndex].Value.set(Float(0.0), Float(0.0), Float(0.0));
            mBackbuffer[pixelIndex].Count = 0;
        }
    }
}

void LDRFilm::FlushTo(const AccumulatedSpectrum& Spectrum, uint32_t Row, uint32_t Column, unsigned char* CanvasDataPtr, int linePitch)
{
    uint32_t CanvasOffset = Row * linePitch + Column * 3;
    Float InvNumSample = Float(1) / Spectrum.Count;
    for (int SpectrumComponentIndex = 2; SpectrumComponentIndex >= 0; SpectrumComponentIndex--)
    {
        Float sRGB = LinearToGamma22Corrected(Spectrum.Value[SpectrumComponentIndex] * InvNumSample);
        CanvasDataPtr[CanvasOffset++] = math::floor2<unsigned char>(math::saturate(sRGB) * Float(256.0) - Float(0.0001));
    }
}
