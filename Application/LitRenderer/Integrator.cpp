#include "Integrator.h"
#include "Integrator.h"

math::vector3<F> PathIntegrator::EvaluateLi(Scene& scene, const math::ray3d<F>& cameraRay, const SurfaceIntersection& recordP1)
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
    SurfaceIntersection hitRecord = recordP1;
    for (int bounce = 0; hitRecord && !math::near_zero(beta) && bounce < MaxBounces; ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        const IMaterial& material = *surface.Material;
        const math::nvector3<F>& N = hitRecord.SurfaceNormal;
        const math::nvector3<F> W_o = -ray.direction();
        F biasedDistance = math::max2<F>(hitRecord.Distance, F(0));
        math::point3d<F> P_i = ray.calc_offset(biasedDistance);
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
            if (!material.Scattering(epsilon, P_i, N, ray, hitRecord.IsOnSurface, scatterLight))
            {
                break;
            }

            ray = scatterLight.scattering;
            const math::nvector3<F>& W_i = ray.direction();
            F pdf = material.pdf(N, W_o, W_i);
            beta *= scatterLight.f * math::saturate(scatterLight.cosine) / pdf;
        }

        if (bounce > 3)
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
