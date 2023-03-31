#include <assert.h>
#include "Integrator.h"

Spectrum PathIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum BackgroundColor = Spectrum::zero();
        return BackgroundColor;
    }

    // Seen light source directly;
    if (recordP1.Object->LightSource != nullptr)
    {
        return recordP1.Object->LightSource->Le();
    }

    const unsigned int MaxBounces = 10;
    float rrContinueProbability = 1.0f;
    auto CheckRussiaRoulette = [&](float prob) { return TerminateSampler.value() > prob; };

    Spectrum Lo = Spectrum::zero();
    Spectrum beta = Spectrum::one();
    Ray viewRay = cameraRay;

    // First hit, on P1
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
            return Weight_BSDF > Float(0);
        }

        Float Weight_BSDF = Float(0);
        LightSource* Light = nullptr;
        bool IsMirrorReflection = false;
    } lastMISRecord;

    for (int bounce = 0; hitRecord && bounce < MaxBounces && !math::near_zero(beta); ++bounce)
    {
        const SceneObject& surface = *hitRecord.Object;
        if (surface.LightSource != nullptr)
        {
            //Note:
            // 
            // Mitsuba will sample random lightsource
            // while PBR-v3 will sample the same light.
            // I have no idea which is better.
            // 
            if (lastMISRecord.IsValid(surface.LightSource.get()))
            {
                Spectrum Le = surface.LightSource->Le();
                Lo += beta * Le * lastMISRecord.Weight_BSDF;
            }
            break;
        }

        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const Direction& T = hitRecord.SurfaceTangent;
        const Direction& N = hitRecord.SurfaceNormal;
        const UVW uvw(N, T);

        //Multiple Importance Sampling
        {
            const std::unique_ptr<Material>& material = surface.Material;
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
            const Point Pi = viewRay.calc_offset(biasedDistance);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            lastMISRecord.IsMirrorReflection = (bsdf.BSDFMask& BSDFMask::MirrorMask) != 0;
            lastMISRecord.Light = nullptr;

            //Sampling Direct Illumination
            if (!lastMISRecord.IsMirrorReflection)
            {
                SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
                if (lightSource != nullptr && lightSource != hitRecord.Object)
                {
                    const Point Pi_1 = lightSource->SampleRandomPoint(u);
                    const Ray lightRay(Pi, Pi_1);

                    SurfaceIntersection recordPi_1 = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                    if (recordPi_1.Object == lightSource)
                    {
                        const Direction& N_light = recordPi_1.SurfaceNormal;
                        const Direction& Wi = uvw.world_2_local(lightRay.direction());
                        const Direction Wi_light = -lightRay.direction();

                        Float NdotV_light = math::dot(N_light, Wi_light);
                        const Float NdotL = CosTheta(Wi);
                        if (NdotV_light > Float(0) && NdotL > Float(0))
                        {
                            const Float pdf_light = scene.SampleLightPdf(lightRay);
                            assert(pdf_light > Float(0));
                            const Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                            const Float weight_mis = PowerHeuristic(pdf_light, pdf_bsdf);
                            const Spectrum f = material->SampleF(Wo, Wi);
                            const Spectrum& Le = lightSource->LightSource->Le();
                            //                      f * cos(Wi)
                            // beta * mis_weight * -------------
                            //                       pdf_light
                            Lo += (weight_mis * NdotL / pdf_light) * (beta * f * Le);
                            lastMISRecord.Light = lightSource->LightSource.get();
                        }
                    }
                }
            }

            //Sampling BSDF
            {
                const Direction Wi = bsdf.SampleWi(u, Wo);
                const Float NdotL = CosTheta(Wi);
                if (NdotL <= Float(0))
                {
                    break;
                }

                viewRay.set_origin(Pi);
                viewRay.set_direction(uvw.local_2_world(Wi));

                const Float pdf_light = scene.SampleLightPdf(viewRay);
                const Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                lastMISRecord.Weight_BSDF = (lastMISRecord.IsMirrorReflection) ? Float(1) : PowerHeuristic(pdf_bsdf, pdf_light);
                const Spectrum f = material->SampleF(Wo, Wi);
                beta *= (NdotL / pdf_bsdf) * f;
            }
        }

        // Termination Check
        if (bounce > 3)
        {
            rrContinueProbability *= 0.95f;
            if (CheckRussiaRoulette(rrContinueProbability))
            {
                break;
            }
            beta /= rrContinueProbability;
        }

        // Find next path ends with Pi+1
        hitRecord = scene.DetectIntersecting(viewRay, nullptr, math::SMALL_NUM<Float>);
    }

    return Lo;
}

Spectrum DebugIntegrator::EvaluateLi(Scene& scene, const Ray& cameraRay, const SurfaceIntersection& recordP1)
{
    if (!recordP1)
    {
        const Spectrum kBackgroundColor = Spectrum::zero();
        return kBackgroundColor;
    }

    const SurfaceIntersection& hitRecord = recordP1;
    const Ray& viewRay = cameraRay;
    const SceneObject& surface = *hitRecord.Object;

    if (surface.LightSource != nullptr)
    {
        Spectrum Le = surface.LightSource->Le();
        return Le;
    }
    else //Sampling BSDF
    {
        Float u[3] =
        {
            mUniformSamplers[0].value(),
            mUniformSamplers[1].value(),
            mUniformSamplers[2].value()
        };

        const Direction& T = hitRecord.SurfaceTangent;
        const Direction& N = hitRecord.SurfaceNormal;
        const UVW uvw(N, T);

        const std::unique_ptr<Material>& material = surface.Material;
        if (material)
        {
            const Float biasedDistance = math::max2<Float>(hitRecord.Distance, Float(0));
            const Point P_i = viewRay.calc_offset(biasedDistance);
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            const Direction Wi = bsdf.SampleWi(u, Wo);
            const Float NdotL = math::dot(N, Wi);

            return Spectrum(-NdotL);
            return Wi * 0.5 + 0.5;

            if (NdotL > 0)
            {
                //auto f = material->SampleF(N, Wo, Wi);
                //auto pdf = material->SamplePdf(N, Wo, Wi);
                //auto r = f * NdotL / pdf;
                //return r;
                return cameraRay.direction() * 0.5 + 0.5;

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
        const Ray& viewRay = cameraRay;

        const SceneObject& surface = *hitRecord.Object;
        if (surface.LightSource != nullptr)
        {
            Spectrum Le = surface.LightSource->Le();
            return Le;
        }
        else
        {
            Spectrum Weights = Spectrum::zero();

            const Float biasedDistance = math::clamp0(hitRecord.Distance);
            const Point P_i = viewRay.calc_offset(biasedDistance);
            Float u[3] =
            {
                mUniformSamplers[0].value(),
                mUniformSamplers[1].value(),
                mUniformSamplers[2].value()
            };

            const Direction& T = hitRecord.SurfaceTangent;
            const Direction& N = hitRecord.SurfaceNormal;
            const UVW uvw(N, T);
            const Direction Wo = uvw.world_2_local(-viewRay.direction());

            const std::unique_ptr<Material>& material = surface.Material;
            const BSDF& bsdf = *material->GetRandomBSDFComponent(u[0]);
            const bool bIsMirrorReflection = (bsdf.BSDFMask & BSDFMask::MirrorMask) != 0;

            if (!bIsMirrorReflection)
            {
                SceneObject* lightSource = scene.UniformSampleLightSource(u[0]);
                if (lightSource != nullptr)
                {
                    Point P_i_1 = lightSource->SampleRandomPoint(u);
                    Ray lightRay(P_i, P_i_1);
                    const Direction Wi_light = -lightRay.direction();
                    const Direction Wi = uvw.world_2_local(lightRay.direction());

                    SurfaceIntersection recordPi_1 = scene.DetectIntersecting(lightRay, nullptr, math::SMALL_NUM<Float>);
                    bool bIsVisible = recordPi_1.Object == lightSource;
                    if (bIsVisible)
                    {
                        Direction N_light = recordPi_1.SurfaceNormal;
                        Float cosThetaPrime = math::dot(N_light, Wi_light);
                        bIsVisible = cosThetaPrime > math::SMALL_NUM<Float> || (cosThetaPrime < -math::SMALL_NUM<Float> && lightSource->IsDualface());

                        if (bIsVisible)
                        {
                            Float pdf_light = scene.SampleLightPdf(lightRay);
                            assert(pdf_light > Float(0));
                            Float pdf_bsdf = material->SamplePdf(Wo, Wi);
                            Float weight = PowerHeuristic(pdf_light, pdf_bsdf);
                            Weights.y = math::saturate(weight);
                        }
                    }
                }
            }

            {
                const Direction Wi = bsdf.SampleWi(u, Wo);
                const Float NdotL = math::dot(N, Wi);
                if (NdotL > Float(0))
                {
                    Float weight = Float(1);
                    if (bIsMirrorReflection)
                    {
                        Float pdf_light = scene.SampleLightPdf(Ray{ P_i, uvw.local_2_world(Wi) });
                        if (pdf_light > Float(0))
                        {
                            Float pdf_bsdf = material->SamplePdf(Wo, Wi);
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
