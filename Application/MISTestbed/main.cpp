#include <iostream>
#include <random>


std::random_device rdevice;
std::mt19937 generator(rdevice());

const float kPI = 3.1415926535f;
const float kUpperBound = kPI * 0.5f;
const float kLowerBound = 0.0f;
const int kSamples = 500;

template<typename Float>
float Trapezoid(Float f)
{
    float sum = 0.0f;
    const float dx = 0.001f;
    for (float x = kLowerBound; x < kUpperBound; x += dx)
    {
        float height = 0.5f * (f(x + dx) + f(x));
        sum += height * dx;
    }
    return sum;
}

template<typename Float>
float UniformSampling(Float f)
{
    std::uniform_real_distribution<float> distribution(kLowerBound, kUpperBound);

    float sum = 0.0f;
    float pdf = 1.0f / (kUpperBound - kLowerBound);
    for (int i = 0; i < kSamples; i++)
    {
        float x = distribution(generator);
        sum += f(x);
    }
    return sum / (kSamples * pdf);
}

template<typename Float, typename PDF, typename Distribution>
float ImportanceSampling(Float f, PDF pdf, Distribution distrib)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);

    float sum = 0.0f;
    for (int i = 0; i < kSamples; i++)
    {
        float epsilon = u(generator);
        float x = distrib(epsilon);
        sum += f(x) / pdf(x);
    }

    return sum / kSamples;
}

template<typename Float
    , typename PDF1, typename Distribution1
    , typename PDF2, typename Distribution2>
    float MultipleImportanceSampling(Float f,
        PDF1 pdf1, Distribution1 distrib1,
        PDF2 pdf2, Distribution2 distrib2)
{
    auto weight = [](float pdf1, float pdf2)
    {
        pdf1 *= pdf1; pdf2 *= pdf2;
        return pdf1 / (pdf1 + pdf2);
    };

    std::uniform_real_distribution<float> u1(0.0f, 1.0f);
    std::uniform_real_distribution<float> u2(0.0f, 1.0f);

    float sum = 0.0f;
    for (int i = 0; i < kSamples; i++)
    {
        //sample pdf 1;
        float epsilon1 = u1(generator);
        float x1 = distrib1(epsilon1);

        sum += weight(pdf1(x1), pdf2(x1)) * f(x1) / pdf1(x1);

        //sample pdf 2;
        float epsilon2 = u2(generator);
        float x2 = distrib2(epsilon2);

        sum += weight(pdf1(x2), pdf2(x2)) * f(x2) / pdf2(x2);
    }

    return sum / kSamples;
}


void main()
{
#if 0
    auto y = [](float x) { return 2.0f + (x - 2.0f) * x * x; };
    auto Fy = [](float x) { return x * (x * x * (x / 4.0f - 2.0f / 3.0f) + 2.0f); };
    auto pdf1 = [](float x) { return x * x * x / 4.0f; };
    auto pdf2 = [](float x) { return 1.0f - 0.5f * x; };
    auto distrib1 = [](float e) { return sqrt(sqrt(e)) * 2.0f; };
    auto distrib2 = [](float e) { return 2.0f * (1.0f - sqrt(1.0f - e)); };
#else

    auto y = [](float x) { return cosf(x) * sinf(x); };
    auto Fy = [](float x) { return 0.5f * sinf(x) * sinf(x); };
    auto pdf1 = [](float x) { return 8.0f * x / (kPI * kPI); };
    auto pdf2 = [](float x) { return 4.0f / kPI - 8 * x / (kPI * kPI); };
    auto distrib1 = [](float e) { return kPI * 0.5f * sqrt(e); };
    auto distrib2 = [](float e) { return kPI * 0.5f * (1.0f - sqrt(1 - e)); };
#endif

    float analytic = Fy(kUpperBound) - Fy(kLowerBound);
    float trapezoid = Trapezoid(y);
    float uniform = UniformSampling(y);
    float importance1 = ImportanceSampling(y, pdf1, distrib1);
    float importance2 = ImportanceSampling(y, pdf2, distrib2);
    float mult_importance = MultipleImportanceSampling(y, pdf1, distrib1, pdf2, distrib2);


    std::cout
        << "analytic result=" << analytic << std::endl
        << "trapezoid sum=" << trapezoid << std::endl
        << "uniform sampling=" << uniform << std::endl
        << "importance sampling =" << importance1 << std::endl
        << "importance sampling =" << importance2 << std::endl
        << "multiple importance sampling =" << mult_importance << std::endl;
}
