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
    const uint32_t BSDFMask;
    math::vector3<F> Albedo = math::vector3<F>::one();

    BSDF(uint32_t mask) : BSDFMask(mask) { }
    BSDF(uint32_t mask, F r, F g, F b) : BSDFMask(mask), Albedo(r, g, b) { }
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

private:
    std::vector<std::unique_ptr<BSDF>> mBSDFComponents;
    uint32_t mBSDFMask = 0;
};

struct Lambertian : public virtual BSDF
{
    Lambertian() : BSDF(Material::BSDFMask::Diffuse) { }
    Lambertian(F r, F g, F b) : BSDF(Material::BSDFMask::Diffuse, r, g, b) { }
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

struct OrenNayer : public virtual Lambertian
{
    math::radian<F> Sigma = math::radian<F>(0);
    math::radian<F> SigmaSquare = math::radian<F>(0);
    F A = F(1);
    F B = F(0);
    OrenNayer() = default;
    OrenNayer(F r, F g, F b, math::radian<F> sigma = math::radian<F>(0));
    virtual math::vector3<F> f(
        const math::nvector3<F>& N,
        const math::nvector3<F>& Wo,
        const math::nvector3<F>& Wi,
        bool IsOnSurface) const override;
};

struct Metal : public BSDF
{
    F Fuzzy = F(0);

    Metal() : BSDF(Material::BSDFMask::Specular) { }
    Metal(F f) : BSDF(Material::BSDFMask::Specular), Fuzzy(f) { }
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

struct Glossy : public Lambertian
{
    F RefractiveIndex = F(2.5);
    F SpecularSamplingProbability = F(0.5);
    F DiffuseSamplingProbability = F(1) - SpecularSamplingProbability;

    Glossy() : BSDF(Material::BSDFMask::Specular) { }
    Glossy(F r, F g, F b, F ior = F(1.5)) : BSDF(Material::BSDFMask::Specular, r, g, b), RefractiveIndex(ior) { }
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

    GGX() : BSDF(Material::BSDFMask::Specular) { }
    GGX(F rf, F ior = F(1.5)) : BSDF(Material::BSDFMask::Specular), Roughness(rf), RefractiveIndex(ior) { }
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

struct Dielectric : public BSDF
{
    F RefractiveIndex = F(1.0003);

    Dielectric() : BSDF(Material::BSDFMask::Diffuse) { }
    Dielectric(F ior) : BSDF(Material::BSDFMask::Diffuse), RefractiveIndex(ior) { }
    virtual bool Scattering(F epsilon[3], const math::vector3<F>& P, const math::nvector3<F>& N, const math::ray3d<F>& Ray, bool IsOnSurface, LightRay& outLightRay) const override;
};

