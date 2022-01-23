#pragma once
#include "PredefinedConstantValues.h"
#include "Vector.h"

namespace math
{
    template<typename value_type> struct radian;
    template<typename value_type> struct degree;

    namespace math_impl
    {
        template<class value_type> constexpr value_type r2d_factor = value_type(180) / PI<value_type>;
        template<class value_type> constexpr value_type d2r_factor = PI<value_type> / value_type(180);
    }

    template<typename value_type>
    struct radian
    {
        value_type value = value_type(0);

        constexpr radian() = default;
        constexpr explicit radian(value_type v) : value(v) { }
        constexpr radian(const degree<value_type>& d) : value(d.value * math_impl::d2r_factor<value_type>) { }
    };

    template<typename value_type>
    struct degree
    {
        value_type value = value_type(0);

        constexpr degree() = default;
        constexpr explicit degree(value_type v) : value(v) { }
        constexpr degree(const radian<value_type>& r) : value(r.value * math_impl::r2d_factor<value_type>) { }
    };

    template<typename value_type>
    struct quaternion
    {
        vector_t<value_type, EDim::_3> v = vector_t<value_type, EDim::_3>(value_type(0), value_type(0), value_type(0));
        value_type w = value_type(1);

        constexpr quaternion() = default;
        constexpr quaternion(const quaternion& q) : w(q.w), v(q.v) { }
        inline quaternion(const vector_t<value_type, EDim::_3>& axis, const radian<value_type>& r);
        constexpr quaternion(value_type _w, vector_t<value_type, EDim::_3> _v) : w(_w), v(_v) { }
        constexpr quaternion(const vector_t<value_type, EDim::_4>& v4) : w(v4.w), v(v4.x, v4.y, v4.z) { }
        constexpr quaternion(value_type _w, value_type _x, value_type _y, value_type _z) : w(_w), v(_x, _y, _z) { }
        constexpr quaternion& operator=(const quaternion& rhs)
        {
            w = rhs.w;
            v = rhs.v;
            return *this;
        }

        // static utilities
        static constexpr quaternion identity() { return quaternion(value_type(1), value_type(0), value_type(0), value_type(0)); }
        static constexpr quaternion identity_neg() { return quaternion(-value_type(1), value_type(0), value_type(0), value_type(0)); }
    };

    // operations	
    template<typename value_type>
    constexpr bool operator==(const radian<value_type>& l, const radian<value_type>& r)
    {
        return &l == &r || is_equal(l.value, r.value);
    }

    template<typename value_type>
    constexpr bool operator!=(const radian<value_type>& l, const radian<value_type>& r)
    {
        return !(l == r);
    }

    template<typename value_type>
    constexpr const radian<value_type>& operator+(const radian<value_type>& r)
    {
        return r;
    }

    template<typename value_type>
    constexpr radian<value_type> operator-(const radian<value_type>& r)
    {
        return radian<value_type>(-r.value);
    }

    template<typename value_type>
    constexpr bool operator<(const radian<value_type>& l, const radian<value_type>& r)
    {
        return l.value < r.value;
    }

    template<typename value_type>
    constexpr bool operator<=(const radian<value_type>& l, const radian<value_type>& r)
    {
        return l.value <= r.value;
    }

    template<typename value_type>
    constexpr bool operator>(const radian<value_type>& l, const radian<value_type>& r)
    {
        return l.value > r.value;
    }

    template<typename value_type>
    constexpr bool operator>=(const radian<value_type>& l, const radian<value_type>& r)
    {
        return l.value >= r.value;

    }

    template<typename value_type>
    constexpr radian<value_type> operator+(const radian<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l.value + r.value);

    }

    template<typename value_type>
    constexpr radian<value_type> operator-(const radian<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l.value - r.value);
    }

    template<typename value_type>
    constexpr radian<value_type> operator*(const radian<value_type>& l, scaler_t<value_type> r)
    {
        return radian<value_type>(l.value * r);

    }
    template<typename value_type>
    constexpr radian<value_type> operator*(scaler_t<value_type> l, const radian<value_type>& r)
    {
        return r * l;
    }

    template<typename value_type>
    constexpr radian<value_type> operator/(const radian<value_type>& l, scaler_t<value_type> r)
    {
        return l * (value_type(1) / r);
    }


    template<typename value_type>
    constexpr radian<value_type>& operator+=(radian<value_type>& l, const radian<value_type>& r)
    {
        l.value += r.value;
        return l;
    }

    template<typename value_type>
    constexpr radian<value_type>& operator-=(radian<value_type>& l, const radian<value_type>& r)
    {
        l.value -= r.value;
        return l;
    }

    template<typename value_type>
    constexpr radian<value_type>& operator*=(radian<value_type>& l, scaler_t<value_type> r)
    {
        l.value *= r;
        return l;
    }

    template<typename value_type>
    constexpr radian<value_type>& operator/=(radian<value_type>& l, scaler_t<value_type> r)
    {
        return l *= (value_type(1) / r);
    }

    // functions
    template<typename value_type>
    inline value_type sin(const radian<value_type>& r)
    {
        return ::sin(r.value);
    }

    template<typename value_type>
    inline value_type cos(const radian<value_type>& r)
    {
        return ::cos(r.value);
    }

    template<typename value_type>
    inline value_type tan(const radian<value_type>& r)
    {
        return ::tan(r.value);
    }

    template<typename value_type>
    inline void normalize(radian<value_type>& r)
    {
        if (r.value < value_type(0))
        {
            value_type vtrunc = ceil(-r.value / constants<value_type>::pi2);
            r.value += vtrunc * constants<value_type>::pi2;
        }
        else
        {
            r.value = fmod(r.value, constants<value_type>::pi2);
        }
    }

    template<typename value_type>
    inline radian<value_type> normalized(const radian<value_type>& r)
    {
        radian<value_type> rst = r;
        normalize(rst);
        return rst;
    }


    // operations
    template<typename value_type>
    constexpr bool operator==(const degree<value_type>& l, const degree<value_type>& r)
    {
        return &l == &r || is_equal(l.value, r.value);
    }

    template<typename value_type>
    constexpr bool operator!=(const degree<value_type>& l, const degree<value_type>& r)
    {
        return !(l == r);
    }

    template<typename value_type>
    constexpr bool operator<(const degree<value_type>& l, const degree<value_type>& r)
    {
        return l.value < r.value;
    }

    template<typename value_type>
    constexpr bool operator<=(const degree<value_type>& l, const degree<value_type>& r)
    {
        return l.value <= r.value;
    }

    template<typename value_type>
    constexpr bool operator>(const degree<value_type>& l, const degree<value_type>& r)
    {
        return l.value > r.value;
    }

    template<typename value_type>
    constexpr bool operator>=(const degree<value_type>& l, const degree<value_type>& r)
    {
        return l.value >= r.value;
    }

    template<typename value_type>
    constexpr const degree<value_type>& operator+(const degree<value_type>& d)
    {
        return d;
    }

    template<typename value_type>
    constexpr degree<value_type> operator-(const degree<value_type>& d)
    {
        return degree<value_type>(-d.value);
    }

    template<typename value_type>
    constexpr degree<value_type> operator+(const degree<value_type>& l, const degree<value_type>& r)
    {
        return degree<value_type>(l.value + r.value);
    }

    template<typename value_type>
    constexpr degree<value_type> operator-(const degree<value_type>& l, const degree<value_type>& r)
    {
        return degree<value_type>(l.value - r.value);
    }

    template<typename value_type>
    constexpr degree<value_type> operator*(const degree<value_type>& l, scaler_t<value_type> r)
    {
        return degree<value_type>(l.value * r);
    }

    template<typename value_type>
    constexpr degree<value_type> operator*(scaler_t<value_type> l, const degree<value_type>& r)
    {
        return r * l;
    }

    template<typename value_type>
    constexpr degree<value_type> operator/(const degree<value_type>& l, scaler_t<value_type> r)
    {
        return l * (value_type(1) / r);
    }

    template<typename value_type>
    constexpr degree<value_type>& operator+=(degree<value_type>& l, const degree<value_type>& r)
    {
        l.value += r.value;
        return l;
    }

    template<typename value_type>
    constexpr degree<value_type>& operator-=(degree<value_type>& l, const degree<value_type>& r)
    {
        l.value -= r.value;
        return l;
    }

    template<typename value_type>
    constexpr degree<value_type>& operator*=(degree<value_type>& l, scaler_t<value_type> r)
    {
        l.value *= r;
        return l;
    }

    template<typename value_type>
    constexpr degree<value_type>& operator/=(degree<value_type>& l, scaler_t<value_type> r)
    {
        return l *= (value_type(1) / r);
    }

    template<class value_type>
    constexpr bool operator==(const degree<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l) == r;
    }

    template<class value_type>
    constexpr bool operator==(const radian<value_type>& l, const degree<value_type>& r)
    {
        return l == radian<value_type>(r);
    }

    template<class value_type>
    constexpr bool operator!=(const degree<value_type>& l, const radian<value_type>& r)
    {
        return !(l == r);
    }

    template<class value_type>
    constexpr bool operator!=(const radian<value_type>& l, const degree<value_type>& r)
    {
        return !(l == r);
    }

    template<class value_type>
    constexpr bool operator<(const degree<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l) < r;
    }

    template<class value_type>
    constexpr bool operator<(const radian<value_type>& l, const degree<value_type>& r)
    {
        return l < radian<value_type>(r);
    }

    template<class value_type>
    constexpr bool operator<=(const degree<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l) <= r;
    }

    template<class value_type>
    constexpr bool operator<=(const radian<value_type>& l, const degree<value_type>& r)
    {
        return l <= radian<value_type>(r);
    }

    template<class value_type>
    constexpr bool operator>(const degree<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l) > r;
    }

    template<class value_type>
    constexpr bool operator>(const radian<value_type>& l, const degree<value_type>& r)
    {
        return l > radian<value_type>(r);
    }

    template<class value_type>
    constexpr bool operator>=(const degree<value_type>& l, const radian<value_type>& r)
    {
        return radian<value_type>(l) >= r;
    }

    template<class value_type>
    constexpr bool operator>=(const radian<value_type>& l, const degree<value_type>& r)
    {
        return l >= radian<value_type>(r);
    }

    template<class value_type>
    constexpr degree<value_type>& operator+=(degree<value_type>& l, const radian<value_type>& r)
    {
        return l += degree<value_type>(r);
    }

    template<class value_type>
    constexpr radian<value_type>& operator+=(radian<value_type>& l, const degree<value_type>& r)
    {
        return l += radian<value_type>(r);
    }

    template<class value_type>
    constexpr degree<value_type>& operator-=(degree<value_type>& l, const radian<value_type>& r)
    {
        return l -= degree<value_type>(r);
    }

    template<class value_type>
    constexpr radian<value_type>& operator-=(radian<value_type>& l, const degree<value_type>& r)
    {
        return l -= radian<value_type>(r);
    }

    // functions
    template<typename value_type>
    inline value_type sin(const degree<value_type>& d)
    {
        return sin(radian<value_type>(d));
    }

    template<typename value_type>
    inline value_type cos(const degree<value_type>& d)
    {
        return cos(radian<value_type>(d));
    }

    template<typename value_type>
    inline value_type tan(const degree<value_type>& d)
    {
        return tan(radian<value_type>(d));
    }

    template<typename value_type>
    inline void normalize(degree<value_type>& d)
    {
        const value_type caps = value_type(360);
        if (d.value < value_type(0))
        {
            d.value += ceil(-d.value / caps) * caps;
        }
        else
        {
            d.value = fmod(d.value, caps);
        }
    }

    template<typename value_type>
    inline degree<value_type> normalized(const degree<value_type>& d)
    {
        degree<value_type> rst = d;
        normalize(rst);
        return rst;
    }

    // oprations
    template<typename value_type>
    constexpr bool operator== (const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return &lhs == &rhs || (is_equal(lhs.w, rhs.w) && lhs.v == rhs.v);
    }

    template<typename value_type>
    constexpr bool operator!= (const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename value_type>
    constexpr const quaternion<value_type>& operator+(const quaternion<value_type>& q)
    {
        return q;
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator-(const quaternion<value_type>& q)
    {
        return quaternion<value_type>(-q.w, -q.v);
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator+(const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return quaternion<value_type>(
            lhs.w + rhs.w,
            lhs.v + rhs.v);
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator-(const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return quaternion<value_type>(
            lhs.w - rhs.w,
            lhs.v - rhs.v);
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator*(const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return quaternion<value_type>(
            lhs.w * rhs.w - dot(lhs.v, rhs.v),
            lhs.w * rhs.v + rhs.w * lhs.v + cross(lhs.v, rhs.v));
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator*(const quaternion<value_type>& lhs, scaler_t<value_type> rhs)
    {
        return quaternion<value_type>(lhs.w * rhs, lhs.v * rhs);
    }

    template<typename value_type>
    constexpr quaternion<value_type> operator*(scaler_t<value_type> lhs, const quaternion<value_type>& rhs)
    {
        return rhs * lhs;
    }

    template<typename value_type>
    constexpr quaternion<value_type>& operator+=(quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        lhs.w += rhs.w;
        lhs.v += rhs.v;
        return lhs;
    }

    template<typename value_type>
    constexpr quaternion<value_type>& operator-=(quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        lhs.w -= rhs.w;
        lhs.v -= rhs.v;
        return lhs;
    }

    template<typename value_type>
    constexpr quaternion<value_type>& operator*=(quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return (lhs = lhs * rhs);
    }

    template<typename value_type>
    constexpr quaternion<value_type>& operator*=(quaternion<value_type>& lhs, scaler_t<value_type> rhs)
    {
        lhs.w *= rhs;
        lhs.v *= rhs;
        return lhs;
    }

    // functions
    template<typename value_type>
    constexpr value_type dot(const quaternion<value_type>& lhs, const quaternion<value_type>& rhs)
    {
        return lhs.w * rhs.w + dot(lhs.v, rhs.v);
    }

    template<typename value_type>
    inline void normalize(quaternion<value_type>& q)
    {
        value_type length2 = magnitude_sqr(q);
        if (!is_equal(length2, value_type(0)) & !is_equal(length2, value_type(1)))
        {
            value_type invLength = value_type(1) / sqrt(length2);
            q.v *= invLength;
            q.w *= invLength;
        }
    }

    template<typename value_type>
    constexpr void conjugate(quaternion<value_type>& q)
    {
        q.v = -q.v;
    }

    template<typename value_type>
    constexpr void invert_w(quaternion<value_type>& q)
    {
        if (q.w < value_type(0))
        {
            q.w = -q.w;
            q.v = -q.v;
        }
    }

    template<typename value_type>
    inline void inverse(quaternion<value_type>& q)
    {
        conjugate(q);
        normalize(q);
    }

    template<typename value_type>
    constexpr value_type magnitude_sqr(quaternion<value_type>& q)
    {
        return dot(q, q);
    }

    template<typename value_type>
    inline value_type magnitude(quaternion<value_type>& q)
    {
        return sqrt(magnitude_sqr(q));
    }

    template<typename value_type>
    constexpr quaternion<value_type> conjugated(const quaternion<value_type>& q)
    {
        quaternion<value_type> rst(q);
        conjugate(rst);
        return rst;
    }

    template<typename value_type>
    inline quaternion<value_type> normalized(const quaternion<value_type>& q)
    {
        quaternion<value_type> rst(q);
        normalize(rst);
        return rst;
    }

    template<typename value_type>
    inline quaternion<value_type> inversed(const quaternion<value_type>& q)
    {
        quaternion<value_type> rst(q);
        inverse(rst);
        return rst;
    }

    template<typename value_type>
    inline quaternion<value_type> slerp(const quaternion<value_type>& s, const quaternion<value_type>& d, value_type f)
    {
        /*
        slerp(Qs, Qd, f) = (Qd * Qs^-1)^f * Qs;
        cross(d,s^-1) means the diff from s to d.

        quat rst = cross(d, s.inversed());
        rst.exp(f);
        return cross(rst, s);

        another slerp:
        slerp(Qs,Qd,f)	=
        (sin((1-t)w) / sinw) * Qs + (sin(tw) / sinw)*Qd =
        (sin((1-t)w)*Qs + sin(tw)*Qd) / sinw
        */

        value_type cosw = dot(s, d);
        value_type f_s, f_e;

        //means sin_w = 0.0
        if (is_equal(cosw, value_type(1)))
        {
            f_s = value_type(1) - f;
            f_e = f;
        }
        else
        {
            value_type sinw = sqrtf(value_type(1) - cosw * cosw);
            value_type inv_sinw = value_type(1) / sinw;
            value_type w = atan2(sinw, cosw);

            f_s = sinf((value_type(1) - f) * w)*  inv_sinw;
            f_e = sinf(f * w) * inv_sinw;
        }

        return f_s * s + f_e * d;
    }


    template<typename value_type>
    inline quaternion<value_type> make_rotation_x_axis(radian<value_type> r)
    {
        return quaternion<value_type>(vector3<value_type>::unit_x(), r);
    }

    template<typename value_type>
    inline quaternion<value_type> make_rotation_y_axis(radian<value_type> r)
    {
        return quaternion<value_type>(vector3<value_type>::unit_y(), r);
    }

    template<typename value_type>
    inline quaternion<value_type> make_rotation_z_axis(radian<value_type> r)
    {
        return quaternion<value_type>(vector3<value_type>::unit_z(), r);
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_3> rotate(const quaternion<value_type>& q, const vector_t<value_type, EDim::_3>& p)
    {
        return (q * quaternion<value_type>(value_type(0), p) * inversed(q)).v;
    }

    template<typename value_type>
    inline vector_t<value_type, EDim::_3> transform(const quaternion<value_type>& q, const vector_t<value_type, EDim::_3>& p)
    {
        return rotate(q, p);
    }

    // Converter
    template<typename value_type>
    constexpr vector_t<value_type, EDim::_4> convert_to_vector4(const quaternion<value_type>& q)
    {
        return vector_t<value_type, EDim::_4>(q.v.x, q.v.y, q.v.z, q.w);
    }

    //template<typename value_type>
    //constexpr euler<value_type> to_eular(const quaternion<value_type>& q)
    //{
    //    vector_t<value_type, EDim::_3> v_sqr2 = q.v * q.v * value_type(2);
    //    value_type xy2 = q.v.x * q.v.y * value_type(2);
    //    value_type yz2 = q.v.y * q.v.z * value_type(2);
    //    value_type zx2 = q.v.z * q.v.x * value_type(2);
    //    value_type wx2 = q.w * q.v.x * value_type(2);
    //    value_type wy2 = q.w * q.v.y * value_type(2);
    //    value_type wz2 = q.w * q.v.z * value_type(2);
    //
    //    return euler<value_type>(
    //        radian<value_type>(atan2(yz2 - wx2, value_type(1) - v_sqr2.x - v_sqr2.y)),
    //        radian<value_type>(asin(clamp<value_type>(-(zx2 + wy2), -value_type(1), value_type(1))),
    //            radian<value_type>(atan2(xy2 - wz2, value_type(1) - v_sqr2.y - v_sqr2.z))
    //            );
    //}

    // implements
    template<typename value_type>
    inline quaternion<value_type>::quaternion(const vector_t<value_type, EDim::_3>& axis, const radian<value_type>& r)
    {
        radian<value_type> halfRadian = r * value_type(0.5);

        value_type halfCos = cos(halfRadian);
        value_type halfSin = sin(halfRadian);

        v = axis * halfSin;
        w = halfCos;

        normalize(*this);
    }

    //template<typename value_type>
    //inline quaternion<value_type> to_quaternion(const euler<value_type>& e)
    //{
    //    vector_t<value_type, EDim::_4> v0
    //    (
    //        cosf(e.yaw.value * value_type(0.5)),
    //        value_type(0),
    //        value_type(0),
    //        sinf(e.yaw.value * value_type(0.5))
    //    );
    //
    //    vector_t<value_type, EDim::_4> v1
    //    (
    //        cosf(e.pitch.value * value_type(0.5)),
    //        value_type(0),
    //        sinf(e.pitch.value * value_type(0.5)),
    //        value_type(0)
    //    );
    //
    //    vector_t<value_type, EDim::_4> v2
    //    (
    //        cosf(e.roll.value * value_type(0.5)),
    //        sinf(e.roll.value * value_type(0.5)),
    //        value_type(0),
    //        value_type(0)
    //    );
    //
    //    return quaternion<value_type>(v0 * v1 * v2);
    //}


    using quat_f = quaternion<float>;
    using quat_d = quaternion<double>;
    using degree_i = degree<int>;
    using radian_f = radian<float>;
    using degree_f = degree<float>;
    using radian_d = radian<double>;
    using degree_d = degree<double>;

}

// user-defined literals
constexpr math::radian<float>  operator"" _radf(long double				r) { return math::radian<float>(static_cast<float>(r)); }
constexpr math::radian<float>  operator"" _radf(unsigned long long int	r) { return math::radian<float>(static_cast<float>(r)); }
constexpr math::radian<double> operator"" _radd(long double				r) { return math::radian<double>(static_cast<double>(r)); }
constexpr math::radian<double> operator"" _radd(unsigned long long int	r) { return math::radian<double>(static_cast<double>(r)); }

constexpr math::degree<float>  operator"" _degf(long double				d) { return math::degree<float>(static_cast<float>(d)); }
constexpr math::degree<float>  operator"" _degf(unsigned long long int	d) { return math::degree<float>(static_cast<float>(d)); }
constexpr math::degree<double> operator"" _degd(long double				d) { return math::degree<double>(static_cast<double>(d)); }
constexpr math::degree<double> operator"" _degd(unsigned long long int	d) { return math::degree<double>(static_cast<double>(d)); }
