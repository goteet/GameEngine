#pragma once
#include "PreInclude.h"
#include "LitRenderer.h"


struct Integrator
{
    virtual ~Integrator() { };
    virtual Spectrum EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1) = 0;
};

class PathIntegrator : public Integrator
{
    random<Float> TerminateSampler;
    random<Float> mUniformSamplers[3];
public:
    virtual Spectrum EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1) override;
};


class DebugIntegrator : public Integrator
{
    random<Float> mUniformSamplers[3];
public:
    virtual Spectrum EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1) override;

};

class MISDebugIntegrator : public Integrator
{
    random<Float> mUniformSamplers[3];
public:
    virtual Spectrum EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1) override;

};
