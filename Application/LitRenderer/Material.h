#pragma once

#include <random>
#include <memory>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>

using F = double;

template<typename T>
struct random
{
    random() : generator(randomDevice()), distribution(F(0), F(1)) { }
    T operator()() { return _value(); }
    T operator()(T range_value) { return _range(range_value); }
    static random<T>& instance() { static random<T> instance; return instance; }
    static T value() { return instance()(); }
    static T range(T range_value) { return instance()(range_value); }

private:
    T _value() { return distribution(generator); }
    T _range(T range_value) { return _value() * T(2) * range_value - range_value; }

    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<F> distribution;
};


F BalanceHeuristic(F pdfA, F pdfB);
F PowerHeuristic(F pdfA, F pdfB);

struct LightRay
{
    bool specular = false;
    F cosine = F(1);
    math::ray3d<F> scattering;
    math::vector3<F> f = math::vector3<F>::zero();
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    F Weight = F(1);

    BSDF(const std::string& debugName, uint32_t mask, F weight) : DebugName(debugName), BSDFMask(mask), Weight(weight) { }
    virtual ~BSDF() { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const = 0;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const
    {
        return math::vector3<F>::zero();
    }
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const
    {
        return F(1);
    }
};

struct Material
{
    enum BSDFMask
    {
        Diffuse = 1 << 0,
        Specular = 1 << 1,
    };

    Material() = default;

    void AddBSDFComponent(std::unique_ptr<BSDF> component);

    bool IsValid() const { return mBSDFComponents.size() > 0; }

    const std::unique_ptr<BSDF>& GetBSDFComponentByIndex(uint32_t index) const { return mBSDFComponents[index]; }

    const std::unique_ptr<BSDF>& GetBSDFComponentByMask(uint32_t mask) const;

    const std::unique_ptr<BSDF>& GetRandomBSDFComponent(F u) const;

    math::vector3<F> SampleF(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const
    {
        math::vector3<F> f = math::vector3<F>::zero();
        for (auto& bsdf : mBSDFComponents)
        {
            f += bsdf->Weight * bsdf->f(N, Wo, Wi, IsOnSurface);
        }
        return f;
    }

    F SamplePdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const
    {
        F pdf = 0;
        for (auto& bsdf : mBSDFComponents)
        {
            pdf += bsdf->pdf(N, Wo, Wi);
        }
        return pdf / static_cast<F>(mBSDFComponents.size());
    }

private:
    std::vector<std::unique_ptr<BSDF>> mBSDFComponents;
    uint32_t mBSDFMask = 0;
};

struct Lambertian : public BSDF
{
    math::vector3<F> Albedo = math::vector3<F>::one();
    Lambertian(F weight = F(1)) : BSDF("Lambertian", Material::BSDFMask::Diffuse, weight) { }
    Lambertian(F r, F g, F b, F weight = F(1)) : BSDF("Lambertian", Material::BSDFMask::Diffuse, weight), Albedo(r, g, b) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

struct OrenNayer : public BSDF
{
    math::vector3<F> Albedo = math::vector3<F>::one();
    math::radian<F> Sigma = math::radian<F>(0);
    math::radian<F> SigmaSquare = math::radian<F>(0);
    F A = F(1);
    F B = F(0);
    OrenNayer(F weight = F(1)) : BSDF("Oren-Nayer", Material::BSDFMask::Diffuse, weight) { }; ;
    OrenNayer(F r, F g, F b, math::radian<F> sigma = math::radian<F>(0), F weight = F(1));
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

struct Glossy : public BSDF
{
    math::vector3<F> Albedo = math::vector3<F>::one();
    F RefractiveIndex = F(2.5);
    F SpecularSamplingProbability = F(0.5);
    F DiffuseSamplingProbability = F(1) - SpecularSamplingProbability;

    Glossy(F weight = F(1)) : BSDF("Glossy-legacy", Material::BSDFMask::Specular, weight) { }
    Glossy(F r, F g, F b, F ior = F(1.5)) : BSDF("Glossy-legacy", Material::BSDFMask::Specular, F(1)), Albedo(r, g, b), RefractiveIndex(ior) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;

private:
    bool IsSpecular(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const;
};

struct GGX : public BSDF
{
    F Roughness = F(0.5);
    F RefractiveIndex = F(2.5);

    GGX(F weight = F(1)) : BSDF("GGX Microfacet", Material::BSDFMask::Specular, weight) { }
    GGX(F roughness, F IoR, F weight = F(1)) : BSDF("GGX Microfacet", Material::BSDFMask::Specular, weight), Roughness(roughness), RefractiveIndex(IoR) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
    virtual F pdf(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi) const override;
};

inline std::unique_ptr<Material> MakeMatteMaterial(F albedoR = F(1), F albedoG = F(1), F albedoB = F(1))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<Lambertian> compLambertian = std::make_unique<Lambertian>(albedoR, albedoG, albedoB);

    material->AddBSDFComponent(std::move(compLambertian));

    return material;
}

inline std::unique_ptr<Material> MakeMatteMaterial(F albedoR, F albedoG, F albedoB, math::radian<F> sigma)
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<OrenNayer> compLambertian = std::make_unique<OrenNayer>(albedoR, albedoG, albedoB, sigma);

    material->AddBSDFComponent(std::move(compLambertian));

    return material;
}

inline std::unique_ptr<Material> MakePlasticMaterial(F Kd, F albedoR, F albedoG, F albedoB, F Ks, F roughness, F IoR = F(1.5))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<Lambertian> compLambertian = std::make_unique<Lambertian>(albedoR, albedoG, albedoB, Kd);
    std::unique_ptr<GGX> compGGX = std::make_unique<GGX>(roughness, IoR, Ks);

    material->AddBSDFComponent(std::move(compLambertian));
    material->AddBSDFComponent(std::move(compGGX));

    return material;
}
