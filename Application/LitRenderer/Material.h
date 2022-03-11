#pragma once

#include <random>
#include <Foundation/Math/Vector.h>
#include <Foundation/Math/Geometry.h>

using F = double;
class Scene;

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


struct IMaterial
{
    virtual ~IMaterial() { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const = 0;
    virtual math::vector3<F> Emitting() const { return math::vector3<F>::zero(); }
};

struct Lambertian : public virtual IMaterial
{
    math::vector3<F> Albedo = math::vector3<F>::one();

    Lambertian() = default;
    Lambertian(F r, F g, F b) : Albedo(r, g, b) { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const override;
};

struct Metal : public IMaterial
{
    F Fuzzy = F(0);

    Metal() = default;
    Metal(F f) : Fuzzy(f) { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const override;
};

struct Glossy : public Lambertian
{
    mutable random<F> RandGenerator;
    F RefractiveIndex = F(2.5);
    F SampleProbability = F(0.5);

    Glossy() = default;
    Glossy(F r, F g, F b, F ior = F(2.5)) : Lambertian(r, g, b), RefractiveIndex(ior) { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const override;
};

struct Dielectric : public IMaterial
{
    F RefractiveIndex = F(1.0003);

    Dielectric() = default;
    Dielectric(F ior) : RefractiveIndex(ior) { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const override;
};

struct PureLight_ForTest : IMaterial
{
    math::vector3<F> Emission = math::vector3<F>::one();

    PureLight_ForTest() = default;
    PureLight_ForTest(F x, F y, F z) : Emission(x, y, z) { }
    virtual bool Scattering(Scene& scene, const math::vector3<F>& P, const math::vector3<F>& N, const math::ray3d<F>& Ray, bool IsFrontFace,
        math::ray3d<F>& outScattering, math::vector3<F>& outBrdf, F& outPdf) const override;
    virtual math::vector3<F> Emitting() const override;
};
