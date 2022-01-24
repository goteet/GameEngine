#pragma
#include <chrono>

struct SimpleTimer
{
    using TimeStampType = std::chrono::high_resolution_clock::time_point;

    SimpleTimer() { Record(); }

    void Record() { mTimeStamp = std::chrono::high_resolution_clock::now(); }

    template<typename Ret>
    Ret ElapsedSpan()
    {
        TimeStampType ts2 = std::chrono::high_resolution_clock::now();
        Ret timeSpan = std::chrono::duration_cast<Ret>(ts2 - mTimeStamp);
        return timeSpan;
    }

    int64_t ElapsedMilliseconds()
    {
        return ElapsedSpan<std::chrono::milliseconds>().count();
    }

private:
    TimeStampType mTimeStamp;
};
