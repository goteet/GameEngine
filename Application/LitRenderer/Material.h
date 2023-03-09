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
    DiffuseMask = 1 << 0,
    SpecularMask = 1 << 1,
    MirrorMask = 1 << 15
};

struct BSDFSample
{
    Direction Wi = Direction::unit_y();
    Spectrum F = Spectrum::zero();
    Float CosineWi = Float(0);
    uint32_t SampleMask = BSDFMask::None;
};

struct DistributionFunction
{
    virtual ~DistributionFunction() = default;
    virtual bool IsNearMirrorReflection() const = 0;
    virtual Direction SampleWh(const Direction& Wo, Float u1, Float u2) const = 0;
    virtual Float D(Float NdotH) const = 0;
    virtual Float G(Float NdotH, Float HdotV, Float HdotL) const = 0;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi)const = 0;
};

struct DistributionGGX : DistributionFunction
{
    Float Roughness;
    Float Alpha, AlphaSquare;
    DistributionGGX(Float roughness = Float(0.5));
    virtual bool IsNearMirrorReflection() const override { return Roughness < Float(0.001); }
    virtual Direction SampleWh(const Direction& Wo, Float u1, Float u2) const override;
    virtual Float D(Float NdotH) const override;
    virtual Float G(Float NdotH, Float HdotV, Float HdotL) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    Float Weight = Float(1);

    BSDF(const std::string& debugName, uint32_t mask, Float weight) : DebugName(debugName), BSDFMask(mask), Weight(weight) { }
    virtual ~BSDF() { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const = 0;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const = 0;
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

    Spectrum SampleF(const Direction& N, const Direction& Wo, const Direction& Wi, bool IsOnSurface) const;

    Float SamplePdf(const Direction& N, const Direction& Wo, const Direction& Wi) const;

private:
    std::vector<std::unique_ptr<BSDF>> mBSDFComponents;
    uint32_t mBSDFMask = 0;
};

struct Lambertian : public BSDF
{
    Spectrum Albedo = Spectrum::one();
    Lambertian(Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::DiffuseMask, weight) { }
    Lambertian(const Spectrum& albedo, Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::DiffuseMask, weight), Albedo(albedo) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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
    OrenNayer(Float weight = Float(1)) : BSDF("Oren-Nayer", BSDFMask::DiffuseMask, weight) { };
    OrenNayer(const Spectrum& albedo, Radian sigma = 0_degd, Float weight = Float(1));
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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

struct TorranceSparrow : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Float RefractiveIndex = Float(2.5);

    TorranceSparrow(std::unique_ptr<DistributionFunction>&& distrib, Float IoR, Float weight = Float(1))
        : BSDF("Torrance-Sparrow"
            , (distrib->IsNearMirrorReflection() ? (BSDFMask::MirrorMask | BSDFMask::SpecularMask) : BSDFMask::SpecularMask)
            , weight)
        , Distribution(std::move(distrib))
        , RefractiveIndex(IoR) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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

struct AshikhminAndShirley : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Float Rd = Float(0.5);
    Float Rs = Float(0.5);
    AshikhminAndShirley(std::unique_ptr<DistributionFunction>&& distrib, Float diffuse, Float specular, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley"
            , (distrib->IsNearMirrorReflection()
                ? (BSDFMask::MirrorMask | BSDFMask::SpecularMask | BSDFMask::DiffuseMask)
                : BSDFMask::SpecularMask | BSDFMask::DiffuseMask)
            , weight)
        , Distribution(std::move(distrib)), Rd(diffuse), Rs(specular) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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

struct AshikhminAndShirleyDiffuse : public BSDF
{
    Float DiffuseWeight;
    AshikhminAndShirleyDiffuse(Float Rd, Float Rs, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley Diffuse", BSDFMask::DiffuseMask, weight)
        , DiffuseWeight((Float(28)* Rd* math::InvPI<Float> / Float(23))* (Float(1) - Rs)) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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

struct AshikhminAndShirleySpecular : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Float Rs = Float(0.5);
    AshikhminAndShirleySpecular(std::unique_ptr<DistributionFunction>&& distrib, Float specular, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley Specular"
            , (distrib->IsNearMirrorReflection() ? (BSDFMask::MirrorMask | BSDFMask::SpecularMask) : BSDFMask::SpecularMask)
            , weight)
        , Distribution(std::move(distrib)), Rs(specular) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, bool IsOnSurface, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
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

using MircofacetGGX = TorranceSparrow;

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
    std::unique_ptr<DistributionGGX> distribGGX = std::make_unique<DistributionGGX>(roughness);
    std::unique_ptr<Lambertian> compLambertian = std::make_unique<Lambertian>(albedo, Kd);
    std::unique_ptr<TorranceSparrow> compGGX = std::make_unique<TorranceSparrow>(std::move(distribGGX), IoR, Ks);

    material->AddBSDFComponent(std::move(compLambertian));
    material->AddBSDFComponent(std::move(compGGX));

    return material;
}

inline std::unique_ptr<Material> MakeMicrofacetGGXMaterialDebug(Float roughness, Float IoR = Float(1.5))
{
    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<DistributionGGX> distribGGX = std::make_unique<DistributionGGX>(roughness);
    std::unique_ptr<TorranceSparrow> compGGX = std::make_unique<TorranceSparrow>(std::move(distribGGX), IoR);

    material->AddBSDFComponent(std::move(compGGX));

    return material;
}

inline std::unique_ptr<Material> MakeAshikhminAndShirleyGGXMaterial(Float roughness, Float Rd, Float Rs)
{

    std::unique_ptr<Material> material = std::make_unique<Material>();
    std::unique_ptr<DistributionGGX> distribGGX = std::make_unique<DistributionGGX>(roughness);

    //std::unique_ptr<AshikhminAndShirley> compGGX = std::make_unique<AshikhminAndShirley>(std::move(distribGGX), Rd, Rs);
    //material->AddBSDFComponent(std::move(compGGX));
    // 
    std::unique_ptr<AshikhminAndShirleyDiffuse> compDiffuse = std::make_unique<AshikhminAndShirleyDiffuse>(Rd, Rs);
    std::unique_ptr<AshikhminAndShirleySpecular> compSpecular = std::make_unique<AshikhminAndShirleySpecular>(std::move(distribGGX), Rs);
    material->AddBSDFComponent(std::move(compDiffuse));
    material->AddBSDFComponent(std::move(compSpecular));

    return material;
}
