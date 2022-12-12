#include "Integrator.h"
#include "Integrator.h"

math::vector3<F> PathIntegrator::EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const HitRecord& recordP1)
{
    if (!recordP1)
    {
        const math::vector3<F> BackgroundColor = math::vector3<F>::zero();
        return BackgroundColor;
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    math::vector3<F> Lo = math::vector3<F>::zero();
    math::vector3<F> beta = math::vector3<F>::one();
    math::ray3d<F> ray = cameraRay;

    // First hit, on p_1
    HitRecord hitRecord = recordP1;
    for (int Bounce = 0; hitRecord && !math::near_zero(beta) && Bounce < MaxBounces; ++Bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        const IMaterial& material = *surface.Material;
        const math::nvector3<F>& N = hitRecord.SurfaceNormal;
        const math::nvector3<F> w_o = -ray.direction();
        F biasedDistance = math::max2<F>(hitRecord.Distance, F(0));
        math::point3d<F> p_i = ray.calc_offset(biasedDistance);
        F epsilon[3] =
        {
            mEpsilonSamplers[0].value(),
            mEpsilonSamplers[1].value(),
            mEpsilonSamplers[2].value()
        };


        math::vector3<F> Le = material.Emitting();
        Lo += beta * Le;

        //Sampling BSDF
        {
            LightRay scatterLight;
            if (!material.Scattering(epsilon, p_i, N, ray, hitRecord.IsOnSurface, scatterLight))
            {
                break;
            }

            ray = scatterLight.scattering;
            const math::nvector3<F>& w_i = ray.direction();
            F pdf = material.pdf(N, w_o, w_i);
            beta *= scatterLight.f * math::saturate(scatterLight.cosine) / pdf;
        }

        if (Bounce > 3)
        {
            rrContinueProbability *= 0.95f;
            if (CheckRussiaRoulette(rrContinueProbability))
            {
                break;
            }
            beta /= rrContinueProbability;
        }

        // Find next path p_i+1
        hitRecord = scene.DetectIntersecting(ray, nullptr, math::SMALL_NUM<F>);
    }

    return Lo;
}
