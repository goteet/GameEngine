#pragma once
#include "Vector.h"

namespace math
{
    enum normalize_hint { norm };
    template<typename value_type, EDim> struct point;
    template<typename value_type, EDim, typename> struct ray;

    template<typename value_type>
    struct point<value_type, EDim::_2> : public vector_t<value_type, EDim::_2>
    {
        constexpr point() = default;
        constexpr point(const point&) = default;
        constexpr point(const vector_t<value_type, EDim::_2>& r) : vector_t<value_type, EDim::_2>(r) { };
        constexpr point(const vector_t<value_type, EDim::_3>& r) : vector_t<value_type, EDim::_3>(r.x, r.y) { };
        constexpr point(value_type x, value_type y) : vector_t<value_type, EDim::_2>(x, y) { }
        constexpr operator vector_t<value_type, EDim::_3>() const { return vector_t<value_type, EDim::_3>(*this, value_type(1)); }

        static constexpr point origin() { return point(value_type(0), value_type(0)); }
    };

    template<typename value_type>
    struct point<value_type, EDim::_3> : public vector_t<value_type, EDim::_3>
    {
        constexpr point() = default;
        constexpr point(const point&) = default;
        constexpr point(const vector_t<value_type, EDim::_3>& r) : vector_t<value_type, EDim::_3>(r) { };
        constexpr point(const vector_t<value_type, EDim::_4>& r) : vector_t<value_type, EDim::_3>(r.x, r.y, r.z) { };
        constexpr point(value_type x, value_type y, value_type z) : vector_t<value_type, EDim::_3>(x, y, z) { }
        constexpr point(const vector_t<value_type, EDim::_2>& r, scaler_t<value_type> z) : vector_t<value_type, EDim::_3>(r, z) { };
        constexpr operator vector_t<value_type, EDim::_4>() const { return vector_t<value_type, EDim::_4>(*this, value_type(1)); }

        static constexpr point origin() { return point_t(value_type(0), value_type(0), value_type(0)); }
    };

    template<typename value_type, EDim dimension, typename = std::enable_if<dimension == EDim::_2 || dimension == EDim::_3>::type>
    struct ray
    {
        ray() = default;
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d) : _origin(o), _direction(normalized(d)) { }
        ray(const point<value_type, dimension>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { }
        ray(const vector_t<value_type, (EDim)(dimension + 1)>& o, const vector_t<value_type, dimension>& d, normalize_hint) : _origin(o), _direction(d) { }
        constexpr const point<value_type, dimension>& origin() const { return _origin; }
        constexpr const vector_t<value_type, dimension>& direction() const { return _direction; }
        constexpr void set_origin(const point<value_type, dimension>& o) { _origin = o; }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir) { _direction = normalized(dir); }
        constexpr void set_direction(const vector_t<value_type, dimension>& dir, normalize_hint) { _direction = dir; }
        constexpr vector_t<value_type, dimension> calc_offset(scaler_t<value_type> length) const { return _origin + _direction * length; }

    private:
        point<value_type, dimension> _origin;
        vector_t<value_type, dimension> _direction;
    };

    template<typename value_type> using point2d = point<value_type, EDim::_2>;
    template<typename value_type> using point3d = point<value_type, EDim::_3>;
    template<typename value_type> using ray2d = ray<value_type, EDim::_2>;
    template<typename value_type> using ray3d = ray<value_type, EDim::_3>;
    using point2df = point2d<float>;
    using point3df = point3d<float>;
    using point2dd = point2d<double>;
    using point3dd = point3d<double>;
    using ray2df = ray2d<float>;
    using ray3df = ray3d<float>;
    using ray2dd = ray2d<double>;
    using ray3dd = ray3d<double>;
}