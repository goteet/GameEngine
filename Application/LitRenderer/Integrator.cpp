#include <assert.h>
#include "Integrator.h"

Spectrum PathIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    bool bSeenLightSourceDirectly = recordP1.Object->LightSource != nullptr;
    if (bSeenLightSourceDirectly)
    {
        return recordP1.Object->LightSource->Le();
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    Spectrum Lo = Spectrum::zero();
    Spectrum beta = Spectrum::one();
    Ray ray = cameraRay;

    // First hit, on p_1
    SurfaceIntersection hitRecord = recordP1;

    struct MISRecord
    {
        bool IsValid(LightSource* light) const
        {
            //Note:
            // Mitsuba will sample random lightsource
            // while PBR-v3 will sample the same light.
            // I have no idea which is better.
            // 
            //return Weight_bsdf > Float(0) && light == Light;
            return Weight_bsdf > Float(0);
        }

        Float Weight_bsdf = Float(0);
        LightSource* Light = nullptr;
        bool IsMirrorReflection = false;
    } lastMISSample;

    for (int bounce = 0; hitRecord && bounce < MaxBounces && !math::near_zero(beta); ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        const bool bIsLightSource = surface.LightSource != nullptr;

        if (bIsLightSource)
        {
            //Note:
            // Mitsuba will sample random lightsource
            // while PBR-v3 will sample the same light.
            // I have no idea which is better.
            // 
            if (lastMISSample.IsValid(surface.LightSource.get()))
            {
                Spectrum Le = surface.LightSource->Le();
                Lo += beta * Le * lastMISSample.Weight_bsdf;
            }
            break;
        }

        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const Direction& N = hitRecord.SurfaceNormal;
        const Direction Wo = -ray.direction();
        const std::unique_ptr<Material>& material = surface.Material;
        const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
        Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
        Point P_i = ray.calc_offset(biasedDistance);

        lastMISSample.IsMirrorReflection = (bsdf.BSDFMask & BSDFMask::MirrorMask) != 0;
        lastMISSample.Light = nullptr;

        //Sampling Light Source
        if (!lastMISSample.IsMirrorReflection)
        {
            SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
            if (lightSource != nullptr && lightSource != hitRecord.Object)
            {
                Point P_i_1 = lightSource->SampleRandomPoint(u);
                Ray lightRay(P_i, P_i_1);
                const Direction& Wi = lightRay.direction();

                SurfaceIntersection lightSI = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                if (lightSI.Object == lightSource)
                {
                    Direction N_light = lightSI.SurfaceNormal;
                    Float cosThetaPrime = math::dot(N_light, -Wi);
                    if (cosThetaPrime > math::SMALL_NUM<Float>)
                    {
                        const Float pdf_light = scene.SampleLightPdf(lightRay);
                        assert(pdf_light > Float(0));
                        const Float pdf_bsdf = material->SamplePdf(N, Wo, Wi);
                        const Float weight_mis = PowerHeuristic(pdf_light, pdf_bsdf);
                        const Float NdotL = math::dot(N, Wi);
                        const Spectrum f = material->SampleF(N, Wo, Wi);
                        const Spectrum& Le = lightSource->LightSource->Le();
                        //                      f * cos(Wi)
                        // beta * mis_weight * -------------
                        //                       pdf_light
                        Lo += (weight_mis * NdotL / pdf_light) * (beta * f * Le);
                        lastMISSample.Light = lightSource->LightSource.get();
                    }
                }
            }
        }

        //Sampling BSDF
        {
            const Direction Wi = bsdf.SampleWi(u, P_i, N, ray);
            const Float NdotL = math::dot(N, Wi);
            if (NdotL <= Float(0))
            {
                break;
            }


            ray.set_origin(P_i);
            ray.set_direction(Wi);

            const Float pdf_light = scene.SampleLightPdf(ray);
            const Float pdf_bsdf = material->SamplePdf(N, Wo, Wi);
            assert(pdf_bsdf > Float(0));
            lastMISSample.Weight_bsdf = (!lastMISSample.IsMirrorReflection)
                ? PowerHeuristic(pdf_bsdf, pdf_light) : Float(1);
            const Spectrum f = material->SampleF(N, Wo, Wi);
            beta *= (NdotL / pdf_bsdf) * f;
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
        hitRecord = scene.DetectIntersecting(ray, nullptr, math::SMALL_NUM<Float>);
    }

    return Lo;
}

Spectrum DebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    const SurfaceIntersection& hitRecord = recordP1;
    const Ray& ray = cameraRay;

    const SceneObject& surface = *hitRecord.Object;
    const std::unique_ptr<Material>& material = surface.Material;
    const Direction& N = hitRecord.SurfaceNormal;
    const Direction Wo = -ray.direction();

    const bool bIsLightSource = surface.LightSource != nullptr;

    if (bIsLightSource)
    {
        Spectrum Le = surface.LightSource->Le();
        return Le;
    }
    else //Sampling BSDF
    {
        Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
        Point P_i = ray.calc_offset(biasedDistance);
        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        if (material)
        {
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const Direction Wi = bsdf.SampleWi(u, P_i, N, ray);
            const Float NdotL = math::dot(N, Wi);
            if (NdotL > 0)
            {
                return Wi;

                const Direction Half = (Wi + Wo);
                return Half * 0.5 + 0.5;
                return Spectrum(1, 0, 0) * (math::dot(N, Half));
            }
        }
    }

    return Spectrum::zero();
}

Spectrum MISDebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    const SurfaceIntersection& hitRecord = recordP1;
    if (hitRecord)
    {
        const Ray& ray = cameraRay;

        const SceneObject& surface = *hitRecord.Object;
        const std::unique_ptr<Material>& material = surface.Material;
        const Direction& N = hitRecord.SurfaceNormal;
        const Direction Wo = -ray.direction();

        const bool bIsLightSource = surface.LightSource != nullptr;

        if (bIsLightSource)
        {
            Spectrum Le = surface.LightSource->Le();
            return Le;
        }
        else
        {
            Spectrum Weights = Spectrum::zero();
            Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
            Point P_i = ray.calc_offset(biasedDistance);
            Float u[3] =
            {
                mUniformSamplers[0].value(),
                mUniformSamplers[1].value(),
                mUniformSamplers[2].value()
            };



            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const bool bIsMirrorReflection = (bsdf.BSDFMask & BSDFMask::MirrorMask) != 0;

            if (!bIsMirrorReflection)
            {
                SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
                if (lightSource != nullptr)
                {
                    Point P_i_1 = lightSource->SampleRandomPoint(u);
                    Ray lightRay(P_i, P_i_1);
                    const Direction& Wi = lightRay.direction();

                    SurfaceIntersection lightSI = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                    bool bIsVisible = lightSI.Object == lightSource;
                    if (bIsVisible)
                    {
                        Direction N_light = lightSI.SurfaceNormal;
                        Float cosThetaPrime = math::dot(N_light, -Wi);
                        bIsVisible = cosThetaPrime > math::SMALL_NUM<Float> || (cosThetaPrime < -math::SMALL_NUM<Float> && lightSource->IsDualface());

                        if (bIsVisible)
                        {
                            Float pdf_light = scene.SampleLightPdf(lightRay);
                            assert(pdf_light > Float(0));
                            Float pdf_bsdf = material->SamplePdf(N, Wo, Wi);
                            Float weight = PowerHeuristic(pdf_light, pdf_bsdf);
                            Weights.y = math::saturate(weight);
                        }
                    }
                }
            }

            {
                const Direction Wi = bsdf.SampleWi(u, P_i, N, ray);
                const Float NdotL = math::dot(N, Wi);
                if (NdotL > Float(0))
                {
                    Float weight = Float(1);
                    if (bIsMirrorReflection)
                    {
                        Float pdf_light = scene.SampleLightPdf(Ray{ P_i, Wi });
                        if (pdf_light > Float(0))
                        {
                            Float pdf_bsdf = material->SamplePdf(N, Wo, Wi);
                            weight = PowerHeuristic(pdf_bsdf, pdf_light);
                        }
                    }
                    Weights.x = math::saturate(weight);
                }
            }
            return Weights;
        }
    }

    return Spectrum::zero();
}
