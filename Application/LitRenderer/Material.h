#pragma once

#include <random>
#include <memory>
#include "PreInclude.h"

template<typename T>
struct random
{
    random() : generator(randomDevice()), distribution(T(0), T(1)) { }
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
    std::uniform_real_distribution<Float> distribution;
};


Float BalanceHeuristic(Float pdfA, Float pdfB);
Float PowerHeuristic(Float pdfA, Float pdfB);

struct BSDFSample
{
    Direction Wi;
    Spectrum F;
    Float cosine = Float(1);
    Ray scattering;
    Spectrum f = Spectrum::zero();
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    Float Weight = Float(1);

    BSDF(const std::string& debugName, uint32_t mask, Float weight) : DebugName(debugName), BSDFMask(mask), Weight(weight) { }
    virtual ~BSDF() { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const = 0;
    virtual bool Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const = 0;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi, bool IsOnSurface) const { return Spectrum::zero(); }
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const { return Float(1); }
};

struct Material
{
    enum BSDFMask
    {
        Diffuse = 1 << 0,
        Specular = 1 << 1,
        Reflection = 1 << 15
    };

    Material() = default;

    void AddBSDFComponent(std::unique_ptr<BSDF> component);

    bool IsValid() const { return mBSDFComponents.size() > 0; }

    const std::unique_ptr<BSDF>& GetBSDFComponentByIndex(uint32_t index) const { return mBSDFComponents[index]; }

    const std::unique_ptr<BSDF>& GetBSDFComponentByMask(uint32_t mask) const;

    const std::unique_ptr<BSDF>& GetRandomBSDFComponent(Float u) const;

    Spectrum SampleF(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi,
        bool IsOnSurface) const
    {
        Spectrum f = Spectrum::zero();
        for (auto& bsdf : mBSDFComponents)
        {
            f += bsdf->Weight * bsdf->f(N, Wo, Wi, IsOnSurface);
        }
        return f;
    }

    Float SamplePdf(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi) const
    {
        Float pdf = 0;
        for (auto& bsdf : mBSDFComponents)
        {
            pdf += bsdf->pdf(N, Wo, Wi);
        }
        return pdf / static_cast<Float>(mBSDFComponents.size());
    }

private:
    std::vector<std::unique_ptr<BSDF>> mBSDFComponents;
    uint32_t mBSDFMask = 0;
};

struct Lambertian : public BSDF
{
    Spectrum Albedo = Spectrum::one();
    Lambertian(Float weight = Float(1)) : BSDF("Lambertian", Material::BSDFMask::Diffuse, weight) { }
    Lambertian(const Spectrum& albedo, Float weight = Float(1)) : BSDF("Lambertian", Material::BSDFMask::Diffuse, weight), Albedo(albedo) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual bool Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual Spectrum f(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi,
        bool IsOnSurface) const override;
    virtual Float pdf(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi) const override;
};

struct OrenNayer : public BSDF
{
    Spectrum Albedo = Spectrum::one();
    Radian Sigma = 0_degd;
    Radian SigmaSquare = 0_degd;
    Float A = Float(1);
    Float B = Float(0);
    OrenNayer(Float weight = Float(1)) : BSDF("Oren-Nayer", Material::BSDFMask::Diffuse, weight) { };
    OrenNayer(const Spectrum& albedo, Radian sigma = 0_degd, Float weight = Float(1));
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual bool Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual Spectrum f(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi,
        bool IsOnSurface) const override;
    virtual Float pdf(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi) const override;
};

struct GGX : public BSDF
{
    Float Roughness = Float(0.5);
    Float RefractiveIndex = Float(2.5);

    GGX(Float weight = Float(1)) : BSDF("GGX Microfacet", Material::BSDFMask::Specular, weight) { }
    GGX(Float roughness, Float IoR, Float weight = Float(1))
        : BSDF("GGX Microfacet"
            , (roughness < 0.05)
            ? (Material::BSDFMask::Reflection | Material::BSDFMask::Specular)
            : Material::BSDFMask::Specular
            , weight)
        , Roughness(roughness)
        , RefractiveIndex(IoR) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual bool Scattering(Float epsilon[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& outLightRay) const override;
    virtual Spectrum f(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi,
        bool IsOnSurface) const override;
    virtual Float pdf(
        const Direction& N,
        const Direction& Wo,
        const Direction& Wi) const override;
};

inline std::unique_ptr<Material> MakeMatteMaterial(const Spectrum& albedo = Spectrum::one())
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<Lambertian> compLambertian = std::make_unique<Lambertian>(albedo);

    material->AddBSDFComponent(std::move(compLambertian));

    return material;
}

inline std::unique_ptr<Material> MakeMatteMaterial(const Spectrum& albedo, Radian sigma)
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<OrenNayer> compLambertian = std::make_unique<OrenNayer>(albedo, sigma);

    material->AddBSDFComponent(std::move(compLambertian));

    return material;
}

inline std::unique_ptr<Material> MakePlasticMaterial(Float Kd, const Spectrum& albedo, Float Ks, Float roughness, Float IoR = Float(1.5))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<Lambertian> compLambertian = std::make_unique<Lambertian>(albedo, Kd);
    std::unique_ptr<GGX> compGGX = std::make_unique<GGX>(roughness, IoR, Ks);

    material->AddBSDFComponent(std::move(compLambertian));
    material->AddBSDFComponent(std::move(compGGX));

    return material;
}

inline std::unique_ptr<Material> MakeGGXMaterialForDebug(Float roughness, Float IoR = Float(1.5))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<GGX> compGGX = std::make_unique<GGX>(roughness, IoR);

    material->AddBSDFComponent(std::move(compGGX));

    return material;
}
