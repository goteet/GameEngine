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

enum BSDFMask
{
    None = 0,
    Diffuse = 1 << 0,
    Specular = 1 << 1,
    Reflection = 1 << 15
};

struct BSDFSample
{
    Direction Wi = Direction::unit_y();
    Spectrum F = Spectrum::zero();
    uint32_t SampleMask = BSDFMask::None;
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    Float Weight = Float(1);

    BSDF(const std::string& debugName, uint32_t mask, Float weight) : DebugName(debugName), BSDFMask(mask), Weight(weight) { }
    virtual ~BSDF() { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const = 0;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi, bool IsOnSurface) const { return Spectrum::zero(); }
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const { return Float(1); }
};

struct Material
{

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
    Lambertian(Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::Diffuse, weight) { }
    Lambertian(const Spectrum& albedo, Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::Diffuse, weight), Albedo(albedo) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
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
    OrenNayer(Float weight = Float(1)) : BSDF("Oren-Nayer", BSDFMask::Diffuse, weight) { };
    OrenNayer(const Spectrum& albedo, Radian sigma = 0_degd, Float weight = Float(1));
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
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

struct TorranceSparrowGTR2 : public BSDF
{
    Float Roughness = Float(0.5);
    Float RefractiveIndex = Float(2.5);

    TorranceSparrowGTR2(Float weight = Float(1)) : BSDF("Microfacet GGX", BSDFMask::Specular, weight) { }
    TorranceSparrowGTR2(Float roughness, Float IoR, Float weight = Float(1))
        : BSDF("GGX Microfacet"
            , (roughness < 0.05)
                ? (BSDFMask::Reflection | BSDFMask::Specular)
                : BSDFMask::Specular
            , weight)
        , Roughness(roughness)
        , RefractiveIndex(IoR) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
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

struct AshikhminAndShirleyGTR2 : public BSDF
{
    Float Roughness = Float(0.5);
    Float Rd = Float(0.5);
    Float Rs = Float(0.5);
    AshikhminAndShirleyGTR2(Float weight = Float(1)) : BSDF("Ashikhmin-Shirley GGX", BSDFMask::Specular, weight) { }
    AshikhminAndShirleyGTR2(Float roughness, Float diffuse, Float specular, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley GGX"
            , (roughness < 0.05)
                ? (BSDFMask::Reflection | BSDFMask::Specular | BSDFMask::Diffuse)
                : BSDFMask::Specular | BSDFMask::Diffuse
            , weight)
        , Roughness(roughness), Rd(diffuse), Rs(specular) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
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

using MircofacetGGX = TorranceSparrowGTR2;

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
    std::unique_ptr<TorranceSparrowGTR2> compGGX = std::make_unique<TorranceSparrowGTR2>(roughness, IoR, Ks);

    material->AddBSDFComponent(std::move(compLambertian));
    material->AddBSDFComponent(std::move(compGGX));

    return material;
}

inline std::unique_ptr<Material> MakeMicrofacetGGXMaterialDebug(Float roughness, Float IoR = Float(1.5))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<TorranceSparrowGTR2> compGGX = std::make_unique<TorranceSparrowGTR2>(roughness, IoR);

    material->AddBSDFComponent(std::move(compGGX));

    return material;
}

inline std::unique_ptr<Material> MakeAshikhminAndShirleyGGXMaterial(Float roughness, Float Rd, Float Rs)
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<AshikhminAndShirleyGTR2> compGGX = std::make_unique<AshikhminAndShirleyGTR2>(roughness, Rd, Rs);

    material->AddBSDFComponent(std::move(compGGX));

    return material;
}
