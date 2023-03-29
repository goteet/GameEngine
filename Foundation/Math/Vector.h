#pragma once
#include <cmath>
#include "PredefinedConstantValues.h"
#include "HelperFunctions.h"

namespace math
{
    enum EDim : size_t
    {
        _2 = 2, _3 = 3, _4 = 4
    };

    template<typename value_type, EDim dimension>
    struct vector_t;

    template<typename value_type>
    struct vector_t<value_type, EDim::_2>
    {
        using index_type = size_t;
        static constexpr EDim dim = EDim::_2;

        union
        {
            char m[dim * sizeof(value_type)];
            value_type v[dim];
            struct { value_type x, y; };
        };

        constexpr vector_t() : x(value_type(0)), y(value_type(0)) { }
        constexpr vector_t(value_type _x, value_type _y) : x(_x), y(_y) { }
        value_type& operator[](index_type idx) { return v[idx]; }
        const value_type& operator[](index_type idx) const { return v[idx]; }
        void set(value_type _x, value_type _y) { x = _x, y = _y; }

        static constexpr vector_t   zero() { return vector_t(value_type(0), value_type(0)); }
        static constexpr vector_t    one() { return vector_t(value_type(1), value_type(1)); }
        static constexpr vector_t unit_x() { return vector_t(value_type(1), value_type(0)); }
        static constexpr vector_t unit_y() { return vector_t(value_type(0), value_type(1)); }
    };

    template<typename value_type>
    struct vector_t<value_type, EDim::_3>
    {
        using index_type = size_t;
        static constexpr EDim dim = EDim::_3;

        union
        {
            char m[dim * sizeof(value_type)];
            value_type v[dim];
            struct { value_type x, y, z; };
        };

        constexpr vector_t() : x(value_type(0)), y(value_type(0)), z(value_type(0)) { }
        constexpr vector_t(value_type _x, value_type _y, value_type _z) : x(_x), y(_y), z(_z) { }
        constexpr vector_t(const vector_t<value_type, EDim::_2>& v2, value_type _z) : vector_t(v2.x, v2.y, _z) { }
        constexpr vector_t(value_type _x, const vector_t<value_type, EDim::_2>& v2) : vector_t(_x, v2.x, v2.y) { }
        value_type& operator[](index_type idx) { return v[idx]; }
        const value_type& operator[](index_type idx) const { return v[idx]; }
        void set(value_type _x, value_type _y, value_type _z) { x = _x, y = _y; z = _z; }
        void set(const vector_t<value_type, EDim::_2>& _v, value_type _z) { set(_v.x, _v.y, _z); }
        void set(value_type _x, const vector_t<value_type, EDim::_2>& _v) { set(_x, _v.y, _v.z); }

        static constexpr vector_t zero() { return vector_t(value_type(0), value_type(0), value_type(0)); }
        static constexpr vector_t one() { return vector_t(value_type(1), value_type(1), value_type(1)); }
        static constexpr vector_t unit_x() { return vector_t(value_type(1), value_type(0), value_type(0)); }
        static constexpr vector_t unit_y() { return vector_t(value_type(0), value_type(1), value_type(0)); }
        static constexpr vector_t unit_z() { return vector_t(value_type(0), value_type(0), value_type(1)); }
        static constexpr vector_t unit_x_neg() { return vector_t(-value_type(1), value_type(0), value_type(0)); }
        static constexpr vector_t unit_y_neg() { return vector_t(value_type(0), -value_type(1), value_type(0)); }
        static constexpr vector_t unit_z_neg() { return vector_t(value_type(0), value_type(0), -value_type(1)); }
    };

    template<typename value_type>
    struct vector_t<value_type, EDim::_4>
    {
        using index_type = size_t;
        static constexpr EDim dim = EDim::_4;

        union
        {
            char m[dim * sizeof(value_type)];
            value_type v[dim];
            struct { value_type x, y, z, w; };
        };

        constexpr vector_t() : x(value_type(0)), y(value_type(0)), z(value_type(0)), w(value_type(0)) { }
        constexpr vector_t(value_type _x, value_type _y, value_type _z, value_type _w) : x(_x), y(_y), z(_z), w(_w) { }
        constexpr vector_t(const vector_t<value_type, EDim::_2>& v2, value_type _z, value_type _w) : vector_t(v2.x, v2.y, _z, _w) { }
        constexpr vector_t(value_type _x, const vector_t<value_type, EDim::_2>& v2, value_type _w) : vector_t(_x, v2.x, v2.y, _w) { }
        constexpr vector_t(value_type _x, value_type _y, const vector_t<value_type, EDim::_2>& _v) : vector_t(_x, _y, _v.x, _v.y) { }
        constexpr vector_t(const vector_t<value_type, EDim::_3>& _v, value_type _w) : vector_t(_v.x, _v.y, _v.z, _w) { }
        constexpr vector_t(value_type _x, const vector_t<value_type, EDim::_3>& _v) : vector_t(_x, _v.x, _v.y, _v.z) { }
        value_type& operator[](index_type idx) { return v[idx]; }
        const value_type& operator[](index_type idx) const { return v[idx]; }
        void set(value_type _x, value_type _y, value_type _z, value_type _w) { x = _x; y = _y; z = _z; w = _w; }
        void set(const vector_t<value_type, EDim::_2>& v2, value_type _z, value_type _w) { set(v2.x, v2.y, _z, _w); }
        void set(value_type _x, const vector_t<value_type, EDim::_2>& v2, value_type _w) { set(_x, v2.x, v2.y, _w); }
        void set(value_type _x, value_type _y, const vector_t<value_type, EDim::_2>& _v) { set(_x, _y, _v.x, _v.y); }
        void set(const vector_t<value_type, EDim::_3>& _v, value_type _w) { set(_v.x, _v.y, _v.z, _w); }
        void set(value_type _x, const vector_t<value_type, EDim::_3>& _v) { set(_x, _v.x, _v.y, _v.z); }

        // static utilities
        static constexpr vector_t unit_zero() { return vector_t(value_type(0), value_type(0), value_type(0), value_type(0)); }
        static constexpr vector_t unit_one() { return vector_t(value_type(1), value_type(1), value_type(1), value_type(1)); }
        static constexpr vector_t unit_x() { return vector_t(value_type(1), value_type(0), value_type(0), value_type(1)); }
        static constexpr vector_t unit_y() { return vector_t(value_type(0), value_type(1), value_type(0), value_type(1)); }
        static constexpr vector_t unit_z() { return vector_t(value_type(0), value_type(0), value_type(1), value_type(1)); }
        static constexpr vector_t axis_x() { return vector_t(value_type(1), value_type(0), value_type(0), value_type(0)); }
        static constexpr vector_t axis_y() { return vector_t(value_type(0), value_type(1), value_type(0), value_type(0)); }
        static constexpr vector_t axis_z() { return vector_t(value_type(0), value_type(0), value_type(1), value_type(0)); }
        static constexpr vector_t unit_x_neg() { return vector_t(-value_type(1), value_type(0), value_type(0), value_type(1)); }
        static constexpr vector_t unit_y_neg() { return vector_t(value_type(0), -value_type(1), value_type(0), value_type(1)); }
        static constexpr vector_t unit_z_neg() { return vector_t(value_type(0), value_type(0), -value_type(1), value_type(1)); }
        static constexpr vector_t axis_x_neg() { return vector_t(-value_type(1), value_type(0), value_type(0), value_type(0)); }
        static constexpr vector_t axis_y_neg() { return vector_t(value_type(0), -value_type(1), value_type(0), value_type(0)); }
        static constexpr vector_t axis_z_neg() { return vector_t(value_type(0), value_type(0), -value_type(1), value_type(0)); }
    };


    template <typename value_type> using vector2 = vector_t<value_type, EDim::_2>;
    template <typename value_type> using vector3 = vector_t<value_type, EDim::_3>;
    template <typename value_type> using vector4 = vector_t<value_type, EDim::_4>;

    using byte2 = vector2<unsigned char>;
    using byte3 = vector3<unsigned char>;
    using byte4 = vector4<unsigned char>;

    using int2 = vector2<int>;
    using int3 = vector3<int>;
    using int4 = vector4<int>;

    using float2 = vector2<float>;
    using float3 = vector3<float>;
    using float4 = vector4<float>;

    using double2 = vector2<double>;
    using double3 = vector3<double>;
    using double4 = vector4<double>;

    template<typename value_type>
    constexpr bool operator==(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return &l == &r || (is_equal(l.x, r.x) && is_equal(l.y, r.y));
    }

    template<typename value_type>
    constexpr bool operator==(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return &l == &r
            || (is_equal(l.x, r.x) &&
                is_equal(l.y, r.y) &&
                is_equal(l.z, r.z));
    }

    template<typename value_type>
    constexpr bool operator==(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return &l == &r
            || (is_equal(l.x, r.x) &&
                is_equal(l.y, r.y) &&
                is_equal(l.z, r.z) &&
                is_equal(l.w, r.w));
    }

    template<typename value_type, EDim dimension>
    constexpr bool operator!=(const vector_t<value_type, dimension>& l, const vector_t<value_type, dimension>& r)
    {
        return !(l == r);
    }

    template<typename value_type, EDim dimension>
    constexpr const vector_t<value_type, dimension>& operator+(const vector_t<value_type, dimension>& _v)
    {
        return _v;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator-(const vector_t<value_type, EDim::_2>& _v)
    {
        return vector_t<value_type, EDim::_2>(-_v.x, -_v.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator-(const vector_t<value_type, EDim::_3>& _v)
    {
        return vector_t<value_type, EDim::_3>(-_v.x, -_v.y, -_v.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator-(const vector_t<value_type, EDim::_4>& v)
    {
        return vector_t<value_type, EDim::_4>(-v.x, -v.y, -v.z, -v.w);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator+(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(l.x + r.x, l.y + r.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator+(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(l.x + r.x, l.y + r.y, l.z + r.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator+(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator+(const vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_2>(l.x + r, l.y + r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator+(const vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_3>(l.x + r, l.y + r, l.z + r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator+(const vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_4>(l.x + r, l.y + r, l.z + r, l.w + r);
    }

    template<typename value_type, EDim dimension>
    constexpr vector_t<value_type, dimension> operator+(scaler_t<value_type> l, const vector_t<value_type, dimension>& r)
    {
        return r + l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator-(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(l.x - r.x, l.y - r.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator-(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(l.x - r.x, l.y - r.y, l.z - r.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator-(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator-(const vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_2>(l.x - r, l.y - r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator-(const vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_3>(l.x - r, l.y - r, l.z - r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator-(const vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_4>(l.x - r, l.y - r, l.z - r, l.w - r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator-(scaler_t<value_type> l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(l - r.x, l - r.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator-(scaler_t<value_type> l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(l - r.x, l - r.y, l - r.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator-(scaler_t<value_type> l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(l - r.x, l - r.y, l - r.z, l - r.w);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator*(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(l.x * r.x, l.y * r.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator*(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(l.x * r.x, l.y * r.y, l.z * r.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator*(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w);
    }
    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> operator*(const vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_2>(l.x * r, l.y * r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> operator*(const vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_3>(l.x * r, l.y * r, l.z * r);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> operator*(const vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        return vector_t<value_type, EDim::_4>(l.x * r, l.y * r, l.z * r, l.w * r);
    }

    template<typename value_type, EDim dimension>
    constexpr vector_t<value_type, dimension> operator*(scaler_t<value_type> l, const vector_t<value_type, dimension>& r)
    {
        return r * l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2>& operator+=(vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        l.x += r.x;
        l.y += r.y;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3>& operator+=(vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        l.x += r.x;
        l.y += r.y;
        l.z += r.z;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4>& operator+=(vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        l.x += r.x;
        l.y += r.y;
        l.z += r.z;
        l.w += r.w;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2>& operator-=(vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        l.x -= r.x;
        l.y -= r.y;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3>& operator-=(vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        l.x -= r.x;
        l.y -= r.y;
        l.z -= r.z;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4>& operator-=(vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        l.x -= r.x;
        l.y -= r.y;
        l.z -= r.z;
        l.w -= r.w;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2>& operator*=(vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        l.x *= r.x;
        l.y *= r.y;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2>& operator*=(vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        l.x *= r;
        l.y *= r;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3>& operator*=(vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        l.x *= r.x;
        l.y *= r.y;
        l.z *= r.z;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3>& operator*=(vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        l.x *= r;
        l.y *= r;
        l.z *= r;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4>& operator*=(vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        l.x *= r.x;
        l.y *= r.y;
        l.z *= r.z;
        l.w *= r.w;
        return l;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4>& operator*=(vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        l.x *= r;
        l.y *= r;
        l.z *= r;
        l.w *= r;
        return l;
    }

    template<typename value_type, EDim dimension, std::enable_if_t<std::is_floating_point<value_type>::value, bool> = true>
    constexpr vector_t<value_type, dimension> operator/(const vector_t<value_type, dimension>& l, scaler_t<value_type> r)
    {
        return l * (value_type(1) / r);
    }

    template<typename value_type, EDim dimension, std::enable_if_t<std::is_floating_point<value_type>::value, bool> = true>
    constexpr vector_t<value_type, dimension>& operator/=(vector_t<value_type, dimension>& l, scaler_t<value_type> r)
    {
        return l *= value_type(1) / r;
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_2> operator/(const vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            return vector_t<value_type, EDim::_2>(l.x / r, l.y / r);
        }
        else
        {
            return vector_t<value_type, EDim::_2>::zero();
        }
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_2>& operator/=(vector_t<value_type, EDim::_2>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            l.x /= r;
            l.y /= r;
        }
        else
        {
            l = vector_t<value_type, EDim::_2>::zero();
        }
        return l;
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_3> operator/(const vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            return vector_t<value_type, EDim::_3>(l.x / r, l.y / r, l.z / r);
        }
        else
        {
            return vector_t<value_type, EDim::_3>::zero();
        }
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_3>& operator/=(vector_t<value_type, EDim::_3>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            l.x /= r;
            l.y /= r;
            l.z /= r;
        }
        else
        {
            l = vector_t<value_type, EDim::_3>::zero();
        }
        return l;
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_4> operator/(const vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            return vector_t<value_type, EDim::_4>(
                l.x / r,
                l.y / r,
                l.z / r,
                l.w / r);
        }
        else
        {
            return vector_t<value_type, EDim::_4>::unit_zero();
        }
    }

    template<typename value_type, std::enable_if_t<std::is_integral<value_type>::value, bool> = true>
    constexpr vector_t<value_type, EDim::_4>& operator/=(vector_t<value_type, EDim::_4>& l, scaler_t<value_type> r)
    {
        if (r != value_type(0))
        {
            l.x /= r;
            l.y /= r;
            l.z /= r;
            l.w /= r;
        }
        else
        {
            l = vector_t<value_type, EDim::_4>::unit_zero();
        }
        return l;
    }

    template<typename value_type>
    constexpr value_type dot(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return l.x * r.x + l.y * r.y;
    }

    template<typename value_type>
    constexpr value_type dot(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return l.x * r.x + l.y * r.y + l.z * r.z;
    }

    template<typename value_type>
    constexpr value_type dot(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
    }

    template<typename value_type>
    constexpr value_type cross(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return l.x * r.y - l.y * r.x;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> cross(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(
            l.y * r.z - l.z * r.y,
            l.z * r.x - l.x * r.z,
            l.x * r.y - l.y * r.x);
    }

    template<typename value_type, EDim dimension>
    constexpr value_type magnitude_sqr(const vector_t<value_type, dimension>& _v)
    {
        return dot(_v, _v);
    }

    template<typename value_type, EDim dimension>
    value_type magnitude(const vector_t<value_type, dimension>& _v)
    {
        return sqrt(magnitude_sqr<value_type, dimension>(_v));
    }

    template<typename value_type, EDim dimension, typename = std::enable_if<dimension == EDim::_2 || dimension == EDim::_3>::type>
    void normalize(vector_t<value_type, dimension>& _v) //Ϊ�˱�����ʽ���ӹ������ô��������������Ϊ void ����
    {
        value_type lengthSqr = magnitude_sqr(_v);
        if (!is_equal(lengthSqr, value_type(0)))
        {
            _v *= value_type(1) / sqrt(lengthSqr);
        }
        else
        {
            _v = vector_t<value_type, dimension>::zero();
        }
    }

    template<typename value_type, EDim dimension, typename = std::enable_if<dimension == EDim::_2 || dimension == EDim::_3>::type>
    constexpr vector_t<value_type, dimension> normalized(const vector_t<value_type, dimension>& _v)
    {
        vector_t<value_type, dimension> rst(_v);
        normalize(rst);
        return rst;
    }

    template<typename value_type, EDim dimension>
    constexpr void inverse(vector_t<value_type, dimension>& _v) //Ϊ�˱�����ʽ���ӹ������ô��������������Ϊ void ����
    {
        value_type lengthSqr = magnitude_sqr(_v);
        if (!is_equal(lengthSqr, value_type(0)))
        {
            _v *= value_type(1) / lengthSqr;
        }
        else
        {
            _v = vector_t<value_type, dimension>::zero();
        }
    }

    template<typename value_type, EDim dimension>
    constexpr vector_t<value_type, dimension> inversed(const vector_t<value_type, dimension>& _v)
    {
        vector_t<value_type, dimension> rst(_v);
        inverse(rst);
        return rst;
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> abs(const vector_t<value_type, EDim::_2>& _v)
    {
        return vector_t<value_type, EDim::_2>(
            _v.x >= value_type(0) ? _v.x : -_v.x,
            _v.y >= value_type(0) ? _v.y : -_v.y);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> abs(const vector_t<value_type, EDim::_3>& _v)
    {
        return vector_t<value_type, EDim::_3>(
            _v.x >= value_type(0) ? _v.x : -_v.x,
            _v.y >= value_type(0) ? _v.y : -_v.y,
            _v.z >= value_type(0) ? _v.z : -_v.z);
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> abs(const vector_t<value_type, EDim::_4>& v)
    {
        return vector_t<value_type, EDim::_4>(
            v.x >= value_type(0) ? v.x : -v.x,
            v.y >= value_type(0) ? v.y : -v.y,
            v.z >= value_type(0) ? v.z : -v.z,
            v.w >= value_type(0) ? v.w : -v.w);
    }



    template<typename value_type> constexpr value_type min_component(const vector_t<value_type, EDim::_2>& v) { return min2(v.x, v.y); }
    template<typename value_type> constexpr value_type max_component(const vector_t<value_type, EDim::_2>& v) { return max2(v.x, v.y); }
    template<typename value_type> constexpr value_type min_component(const vector_t<value_type, EDim::_3>& v) { return min3(v.x, v.y, v.z); }
    template<typename value_type> constexpr value_type max_component(const vector_t<value_type, EDim::_3>& v) { return max3(v.x, v.y, v.z); }
    template<typename value_type> constexpr value_type min_component(const vector_t<value_type, EDim::_4>& v) { return min2(min2(v.x, v.y), min2(v.z, v.w)); }
    template<typename value_type> constexpr value_type max_component(const vector_t<value_type, EDim::_4>& v) { return max2(max2(v.x, v.y), max2(v.z, v.w)); }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> min_comp_wise(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(
            min2(l.x, r.x),
            min2(l.y, r.y));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> min_comp_wise(const vector_t<value_type, EDim::_2>& f0, const vector_t<value_type, EDim::_2>& f1, const vector_t<value_type, EDim::_2>& f2)
    {
        return vector_t<value_type, EDim::_2>(
            min3(f0.x, f1.x, f2.x),
            min3(f0.y, f1.y, f2.y));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> min_comp_wise(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(
            min2(l.x, r.x),
            min2(l.y, r.y),
            min2(l.z, r.z));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> min_comp_wise(const vector_t<value_type, EDim::_3>& f0, const vector_t<value_type, EDim::_3>& f1, const vector_t<value_type, EDim::_3>& f2)
    {
        return vector_t<value_type, EDim::_3>(
            min3(f0.x, f1.x, f2.x),
            min3(f0.y, f1.y, f2.y),
            min3(f0.z, f1.z, f2.z));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> min_comp_wise(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(
            min2(l.x, r.x),
            min2(l.y, r.y),
            min2(l.z, r.z),
            min2(l.w, r.w));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> min_comp_wise(const vector_t<value_type, EDim::_4>& f0, const vector_t<value_type, EDim::_4>& f1, const vector_t<value_type, EDim::_4>& f2)
    {
        return vector_t<value_type, EDim::_4>(
            min3(f0.x, f1.x, f2.x),
            min3(f0.y, f1.y, f2.y),
            min3(f0.z, f1.z, f2.z),
            min3(f0.w, f1.w, f2.w));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> max_comp_wise(const vector_t<value_type, EDim::_2>& l, const vector_t<value_type, EDim::_2>& r)
    {
        return vector_t<value_type, EDim::_2>(
            max2(l.x, r.x),
            max2(l.y, r.y));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_2> max_comp_wise(const vector_t<value_type, EDim::_2>& f0, const vector_t<value_type, EDim::_2>& f1, const vector_t<value_type, EDim::_2>& f2)
    {
        return vector_t<value_type, EDim::_2>(
            max3(f0.x, f1.x, f2.x),
            max3(f0.y, f1.y, f2.y));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> max_comp_wise(const vector_t<value_type, EDim::_3>& l, const vector_t<value_type, EDim::_3>& r)
    {
        return vector_t<value_type, EDim::_3>(
            max2(l.x, r.x),
            max2(l.y, r.y),
            max2(l.z, r.z));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_3> max_comp_wise(const vector_t<value_type, EDim::_3>& f0, const vector_t<value_type, EDim::_3>& f1, const vector_t<value_type, EDim::_3>& f2)
    {
        return vector_t<value_type, EDim::_3>(
            max3(f0.x, f1.x, f2.x),
            max3(f0.y, f1.y, f2.y),
            max3(f0.z, f1.z, f2.z));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> max_comp_wise(const vector_t<value_type, EDim::_4>& l, const vector_t<value_type, EDim::_4>& r)
    {
        return vector_t<value_type, EDim::_4>(
            max2(l.x, r.x),
            max2(l.y, r.y),
            max2(l.z, r.z),
            max2(l.w, r.w));
    }

    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> max_comp_wise(const vector_t<value_type, EDim::_4>& f0, const vector_t<value_type, EDim::_4>& f1, const vector_t<value_type, EDim::_4>& f2)
    {
        return vector_t<value_type, EDim::_4>(
            max3(f0.x, f1.x, f2.x),
            max3(f0.y, f1.y, f2.y),
            max3(f0.z, f1.z, f2.z),
            max3(f0.w, f1.w, f2.w));
    }
}