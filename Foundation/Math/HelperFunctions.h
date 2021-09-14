#pragma once
#include "PredefinedConstantValues.h"

namespace math
{
    template<typename value_type>
    struct helper_identity { using type = value_type; };

    template<typename value_type>
    using scaler_t = typename helper_identity<value_type>::type;

    // Utility Functions
    template<typename value_type>
    constexpr value_type frac(value_type f)
    {
        return f - static_cast<int>(f);
    }

    template<typename value_type>
    constexpr int floor2int(value_type f)
    {
        return static_cast<int>(f);
    }

    template<typename value_type>
    constexpr int ceil2int(value_type f)
    {
        return static_cast<int>(std::ceil(f));
    }

    template<typename value_type>
    constexpr bool is_equal(value_type lhs, value_type rhs)
    {
        return lhs == rhs;
    }

    template<>
    constexpr bool is_equal<float>(float lhs, float rhs)
    {
        float diff = lhs - rhs;
        return lhs == rhs || (diff <  EPSILON<float> && diff > -EPSILON<float>);
    }

    template<>
    constexpr bool is_equal<double>(double lhs, double rhs)
    {
        double diff = lhs - rhs;
        return lhs == rhs || (diff < EPSILON<double> && diff > -EPSILON<double>);
    }

    template<typename value_type>
    constexpr value_type clamp(value_type value, scaler_t<value_type> minValue, scaler_t<value_type> maxValue)
    {
        return value < minValue
            ? minValue
            : (value > maxValue
                ? maxValue
                : value);
    }

    template<typename value_type>
    constexpr value_type clamp(value_type value)
    {
        return clamp(value, value_type(0), value_type(1));
    }

    template<typename value_type>
    constexpr value_type lerp(value_type l, scaler_t<value_type> r, float f)
    {
        return l + (r - l) * f;
    }

    template<typename value_type>
    constexpr value_type lerp(value_type l, scaler_t<value_type> r, double f)
    {
        return l + (r - l) * f;
    }

    template<typename value_type>
    constexpr value_type pow_n(value_type base, size_t iexp)
    {
        return iexp == 0
            ? value_type(1)
            : (iexp == 1
                ? base
                : pow_n(base, iexp - 1) * base);
    }

    template<typename value_type>
    constexpr value_type min2(value_type a, scaler_t<value_type> b)
    {
        return a < b ? a : b;
    }

    template<typename value_type>
    constexpr value_type max2(value_type a, scaler_t<value_type> b)
    {
        return a > b ? a : b;
    }

    template<typename value_type>
    constexpr value_type min3(value_type a, scaler_t<value_type> b, scaler_t<value_type> c)
    {
        return a < b
            ? (a < c ? a : c)
            : (b < c ? b : c);
    }

    template<typename value_type>
    constexpr value_type max3(value_type a, scaler_t<value_type> b, scaler_t<value_type> c)
    {
        return a > b
            ? (a > c ? a : c)
            : (b > c ? b : c);
    }
}