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
    Spectrum Fresnel = Spectrum::zero();
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

struct RefractionIndex
{
    //https://refractiveindex.info/?shelf=3d&book=metals&page=gold
    static const Float AirRefractiveIndex;
    static RefractionIndex Dielectric(Float Nt, Float Ni = AirRefractiveIndex) { return RefractionIndex{ Nt, Float(0), Ni }; }
    static RefractionIndex Conductor(Float Nt, Float Kt, Float Ni = AirRefractiveIndex) { return RefractionIndex{ Nt, Kt, Ni }; }
    static RefractionIndex Plastic(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(1.5), Float(0), Spectrum::one(), Ni }; }
    static RefractionIndex Gold(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(0.270), Float(2.779), Spectrum(0.944,0.776,0.373), Ni }; }
    static RefractionIndex Silver(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(0.150), Float(3.473), Spectrum(0.962,0.949,0.922), Ni }; }
    static RefractionIndex Copper(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(0.461), Float(2.974), Spectrum(0.926,0.721,0.504), Ni }; }
    static RefractionIndex Lead(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(1.908), Float(3.438), Spectrum(0.632,0.626,0.641), Ni }; }
    static RefractionIndex Iron(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(2.930), Float(3.000), Spectrum(0.531,0.512,0.496), Ni }; }
    static RefractionIndex Aluminium(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(1.097), Float(6.794), Spectrum(0.912,0.914,0.920), Ni }; }
    static RefractionIndex Platinum(Float Ni = AirRefractiveIndex) { return RefractionIndex{ Float(2.188), Float(3.929), Spectrum(0.679,0.642,0.588), Ni }; }

    const Float Nt, Kt, Ni;
    const Spectrum R0;

    RefractionIndex(Float Nt, Float Kt = Float(0), Float Ni = AirRefractiveIndex);
    RefractionIndex(Float Nt, Float Kt, const Spectrum& R, Float Ni = AirRefractiveIndex) : Nt(Nt), Kt(Kt), Ni(Ni), R0(R) { }
    bool IsMetal() const { return Kt > Float(0); }
    const Spectrum& SpecularColor() const { return R0; }
    Spectrum Fresnel(Float CosineThetaI) const;
};

struct BSDF
{
    const std::string DebugName;
    const uint32_t BSDFMask;
    Float Weight = Float(1);

    BSDF(const std::string& debugName, uint32_t mask, Float weight) : DebugName(debugName), BSDFMask(mask), Weight(weight) { }
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
    static std::unique_ptr<Material> CreatePlastic(Float Kd, const Spectrum& albedo, Float Ks, Float roughness, const RefractionIndex& fresnel = RefractionIndex::Dielectric(Float(1.5)));
    static std::unique_ptr<Material> CreateAshikhminAndShirley(Float roughness, const Spectrum& Rd, const Spectrum& Rs);
    static std::unique_ptr<Material> CreateMicrofacetGGX_Debug(Float roughness, const RefractionIndex& fresnel);

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
    Lambertian(Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::DiffuseMask, weight) { }
    Lambertian(const Spectrum& albedo, Float weight = Float(1)) : BSDF("Lambertian", BSDFMask::DiffuseMask, weight), Albedo(albedo) { }
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
    OrenNayar(Float weight = Float(1)) : BSDF("Oren-Nayer", BSDFMask::DiffuseMask, weight) { };
    OrenNayar(const Spectrum& albedo, Radian sigma = 0_degd, Float weight = Float(1));
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct TorranceSparrow : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    RefractionIndex Fresnel;
    Float Ks = Float(1);

    TorranceSparrow(std::unique_ptr<DistributionFunction>&& distrib, const RefractionIndex& fresnel, Float s = Float(1), Float weight = Float(1));
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
    AshikhminAndShirley(std::unique_ptr<DistributionFunction>&& distrib, const Spectrum& Rd, const Spectrum& Rs, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley"
            , (distrib->IsNearMirrorReflection()
                ? (BSDFMask::MirrorMask | BSDFMask::SpecularMask | BSDFMask::DiffuseMask)
                : BSDFMask::SpecularMask | BSDFMask::DiffuseMask)
            , weight)
        , Distribution(std::move(distrib)), Rd(Rd), Rs(Rs)
        , DiffuseWeight((Float(28)* math::InvPI<Float> / Float(23)) * (Rd * (Spectrum::one() - Rs))) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct AshikhminAndShirleyDiffuse : public BSDF
{
    Spectrum Rd = Spectrum::one();
    Spectrum DiffuseWeight = (Float(28) / Float(23) * math::InvPI<Float>) * Spectrum::one();
    AshikhminAndShirleyDiffuse(const Spectrum& Rd, const Spectrum& Rs, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley Diffuse", BSDFMask::DiffuseMask, weight), Rd(Rd)
        , DiffuseWeight((Float(28) / Float(23) * math::InvPI<Float> ) * (Rd * (Spectrum::one() - Rs))) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

struct AshikhminAndShirleySpecular : public BSDF
{
    std::unique_ptr<DistributionFunction> Distribution;
    Spectrum Rs = Spectrum::one();
    AshikhminAndShirleySpecular(std::unique_ptr<DistributionFunction>&& distrib, const Spectrum& Rs, Float weight = Float(1))
        : BSDF("Ashikhmin-Shirley Specular"
            , (distrib->IsNearMirrorReflection() ? (BSDFMask::MirrorMask | BSDFMask::SpecularMask) : BSDFMask::SpecularMask)
            , weight)
        , Distribution(std::move(distrib)), Rs(Rs) { }
    virtual bool SampleFCosOverPdf(Float u[3], const Point& P, const Direction& N, const Ray& Ray, BSDFSample& oBSDFSample) const override;
    virtual Direction SampleWi(Float u[3], const Point& P, const Direction& N, const Ray& Ray) const override;
    virtual Spectrum f(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
    virtual Float pdf(const Direction& N, const Direction& Wo, const Direction& Wi) const override;
};

using Mircofacet = TorranceSparrow;
