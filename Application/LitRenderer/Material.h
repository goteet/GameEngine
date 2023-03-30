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

enum BSDFMask : uint32_t
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
    virtual bool IsNearMirrorReflection() const override { return Roughness <= Float(0.05); }
    virtual Direction SampleWh(const Direction& Wo, Float u1, Float u2) const override;
    virtual Float D(Float NdotH) const override;
    virtual Float G(Float NdotH, Float HdotV, Float HdotL) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct RefractionIndexSetting
{
    //https://refractiveindex.info/?shelf=3d&book=metals&page=gold
    static const Float AirRefractiveIndex;
    static RefractionIndexSetting Dielectric(Float Nt, Float Ni = AirRefractiveIndex) { return  RefractionIndexSetting{ Nt, Float(0), Ni }; }
    static RefractionIndexSetting Conductor(Float Nt, Float Kt, Float Ni = AirRefractiveIndex) { return RefractionIndexSetting{ Nt, Kt, Ni }; }
    static RefractionIndexSetting Plastic(Float Ni = AirRefractiveIndex) { return   RefractionIndexSetting{ Float(1.5), Float(0), Ni }; }
    static RefractionIndexSetting Gold(Float Ni = AirRefractiveIndex) { return      RefractionIndexSetting{ Float(0.270), Float(2.779), Ni }; }
    static RefractionIndexSetting Silver(Float Ni = AirRefractiveIndex) { return    RefractionIndexSetting{ Float(0.150), Float(3.473), Ni }; }
    static RefractionIndexSetting Copper(Float Ni = AirRefractiveIndex) { return    RefractionIndexSetting{ Float(0.461), Float(2.974), Ni }; }
    static RefractionIndexSetting Lead(Float Ni = AirRefractiveIndex) { return      RefractionIndexSetting{ Float(1.908), Float(3.438), Ni }; }
    static RefractionIndexSetting Iron(Float Ni = AirRefractiveIndex) { return      RefractionIndexSetting{ Float(2.930), Float(3.000), Ni }; }
    static RefractionIndexSetting Aluminium(Float Ni = AirRefractiveIndex) { return RefractionIndexSetting{ Float(1.097), Float(6.794), Ni }; }
    static RefractionIndexSetting Platinum(Float Ni = AirRefractiveIndex) { return  RefractionIndexSetting{ Float(2.188), Float(3.929), Ni }; }

    const Float Nt, Kt, Ni;
    const Spectrum R0;

    RefractionIndexSetting(Float Nt, Float Kt = Float(0), Float Ni = AirRefractiveIndex);
    bool IsMetal() const { return Kt > Float(0); }
    const Spectrum& SpecularColor() const { return R0; }
    Spectrum Fresnel(Float CosineThetaI) const;
};

namespace SpecularColor
{
    static Spectrum Gold() { return         Spectrum(0.944, 0.776, 0.373); }
    static Spectrum Silver() { return       Spectrum(0.962, 0.949, 0.922); }
    static Spectrum Copper() { return       Spectrum(0.926, 0.721, 0.504); }
    static Spectrum Lead() { return         Spectrum(0.632, 0.626, 0.641); }
    static Spectrum Iron() { return         Spectrum(0.531, 0.512, 0.496); }
    static Spectrum Aluminium() { return    Spectrum(0.912, 0.914, 0.920); }
    static Spectrum Platinum() { return     Spectrum(0.679, 0.642, 0.588); }
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    Float Weight = Float(1);

    BSDF(const std::string& debugName, uint32_t mask) : DebugName(debugName), BSDFMask(mask) { }
    virtual ~BSDF() { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const = 0;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const = 0;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const { return Spectrum::zero(); }
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const { return Float(1); }
};

struct Material
{
    static std::unique_ptr<Material> CreateMatte(const Spectrum& albedo = Spectrum::one());
    static std::unique_ptr<Material> CreateMatte(const Spectrum& albedo, Radian sigma);
    static std::unique_ptr<Material> CreatePlastic(const Spectrum& albedo, Float roughness, const Spectrum& Rs = Spectrum::one());
    static std::unique_ptr<Material> CreateAshikhminAndShirley(Float roughness, const Spectrum& Rd, const Spectrum& Rs);
    static std::unique_ptr<Material> CreateMicrofacetGGX_Debug(Float roughness, const Spectrum& Rs);

    Material() = default;
    void AddBSDFComponent(std::unique_ptr<BSDF> component);
    bool IsValid() const { return mBSDFComponents.size() > 0; }
    const std::unique_ptr<BSDF>& GetBSDFComponentByIndex(uint32_t index) const { return mBSDFComponents[index]; }
    const std::unique_ptr<BSDF>& GetBSDFComponentByMask(uint32_t mask) const;
    const std::unique_ptr<BSDF>& GetRandomBSDFComponent(Float u) const;
    Spectrum SampleF(const Direction& N, const Direction& Wo, const Direction& Wi) const;
    Float SamplePdf(const Direction& N, const Direction& Wo, const Direction& Wi) const;

private:
    std::vector<std::unique_ptr<BSDF>> mBSDFComponents;
    uint32_t mBSDFMask = 0;
};

struct Lambertian : public BSDF
{
    Spectrum Albedo = Spectrum::one();

    Lambertian() : BSDF("Lambertian", BSDFMask::DiffuseMask) { }
    Lambertian(const Spectrum& albedo) : BSDF("Lambertian", BSDFMask::DiffuseMask), Albedo(albedo) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct OrenNayar : public BSDF
{
    Spectrum Albedo = Spectrum::one();
    Radian Sigma = 0_degd;
    Radian SigmaSquare = 0_degd;
    Float A = Float(1);
    Float B = Float(0);

    OrenNayar() : BSDF("Oren-Nayer", BSDFMask::DiffuseMask) { };
    OrenNayar(const Spectrum& albedo, Radian sigma = 0_degd);
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct TorranceSparrow : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Spectrum Rs;

    TorranceSparrow(std::unique_ptr<DistributionFunction>&& distrib, const Spectrum& Rs);
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct AshikhminAndShirley : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Spectrum Rd = Spectrum::one();
    Spectrum Rs = Spectrum::one();
    Spectrum OneMinusRs = Spectrum::zero();
    Spectrum DiffuseWeight = (Float(28) / Float(23) * math::InvPI<Float>) * Spectrum::one();

    AshikhminAndShirley(std::unique_ptr<DistributionFunction>&& distrib, const Spectrum& Rd, const Spectrum& Rs);
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct AshikhminAndShirleyDiffuse : public BSDF
{
    Spectrum Rd = Spectrum::one();
    Spectrum DiffuseWeight = (Float(28) / Float(23) * math::InvPI<Float>) * Spectrum::one();

    AshikhminAndShirleyDiffuse(const Spectrum& Rd, const Spectrum& Rs);
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct AshikhminAndShirleySpecular : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Spectrum Rs = Spectrum::one();

    AshikhminAndShirleySpecular(std::unique_ptr<DistributionFunction>&& distrib, const Spectrum& Rs);
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

using Mircofacet = TorranceSparrow;
