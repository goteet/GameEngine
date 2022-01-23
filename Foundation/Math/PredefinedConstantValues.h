#pragma once

namespace math
{
    namespace math_impl
    {
        template<typename value_type>
        struct epsilon_t
        {
            constexpr static value_type value = value_type(0);
        };

        template<>
        struct epsilon_t<float>
        {
            constexpr static float value = 1e-6f;
        };

        template<>
        struct epsilon_t<double>
        {
            constexpr static double value = 1e-15;
        };

        template<typename value_type>
        struct small_num_t
        {
            constexpr static value_type value = value_type(0);
        };

        template<>
        struct small_num_t<float>
        {
            constexpr static float value = 1e-4f;
        };

        template<>
        struct small_num_t<double>
        {
            constexpr static double value = 1e-8f;
        };
    }

    template<typename value_type>
    struct constant_value
    {
        constexpr static value_type pi = value_type(3.14159265358979323846);
        constexpr static value_type two_pi = pi * value_type(2);
        constexpr static value_type half_pi = pi * value_type(0.5);
        constexpr static value_type epsilon = math_impl::epsilon_t<value_type>::value;
        constexpr static value_type small_num = math_impl::small_num_t<value_type>::value;
    };

    template<typename value_type>
    constexpr value_type PI = constant_value<value_type>::pi;

    template<typename value_type>
    constexpr value_type TWO_PI = constant_value<value_type>::two_pi;

    template<typename value_type>
    constexpr value_type HALF_PI = constant_value<value_type>::half_pi;

    template<typename value_type>
    constexpr value_type EPSILON = constant_value<value_type>::epsilon;

    template<typename value_type>
    constexpr value_type SMALL_NUM = constant_value<value_type>::small_num;
}
