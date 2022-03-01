#pragma once
#include <stdarg.h>
#include <string>

#ifndef USE_CONFSOLE_OUTPUT
#include <debugapi.h>
#endif

namespace engine
{

#ifdef USE_CONFSOLE_OUTPUT
    inline void Print(const char* msg) { printf("%s", msg); }
    inline void Print(const wchar_t* msg) { wprintf(L"%ws", msg); }
#else
    inline void Print(const char* msg) { OutputDebugStringA(msg); }
    inline void Print(const wchar_t* msg) { OutputDebugStringW(msg); }
#endif

    inline void Printf(const char* format, ...)
    {
        char buffer[256];
        va_list ap;
        va_start(ap, format);
        vsprintf_s(buffer, 256, format, ap);
        va_end(ap);
        Print(buffer);
    }

    inline void Printf(const wchar_t* format, ...)
    {
        wchar_t buffer[256];
        va_list ap;
        va_start(ap, format);
        vswprintf(buffer, 256, format, ap);
        va_end(ap);
        Print(buffer);
    }

#ifndef RELEASE
    inline void PrintSubMessage(const char* format, ...)
    {
        Print("--> ");
        char buffer[256];
        va_list ap;
        va_start(ap, format);
        vsprintf_s(buffer, 256, format, ap);
        va_end(ap);
        Print(buffer);
        Print("\n");
    }
    inline void PrintSubMessage(const wchar_t* format, ...)
    {
        Print("--> ");
        wchar_t buffer[256];
        va_list ap;
        va_start(ap, format);
        vswprintf(buffer, 256, format, ap);
        va_end(ap);
        Print(buffer);
        Print("\n");
    }
    inline void PrintSubMessage(void) {}
#endif
}

#ifdef ERROR
#undef ERROR
#endif
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef HALT
#undef HALT
#endif


#ifdef RELEASE

#define ASSERT(isTrue, ...) (void)(isTrue)
#define WARN_ONCE_IF(isTrue, ...) (void)(isTrue)
#define WARN_ONCE_IF_NOT(isTrue, ...) (void)(isTrue)
#define ERROR(msg, ...)
#define DEBUGPRINT(msg, ...) \
	do                       \
	{                        \
	} while (0)
#define ASSERT_SUCCEEDED(hr, ...) (void)(hr)

#else    // !RELEASE

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define STRINGIFY_FILE_LINE(file, line) STRINGIFY_BUILTIN(file) " @ " STRINGIFY_BUILTIN(line)
#define ASSERT(isFalse, ...)                                                                        \
    if (!(isFalse))                                                                                 \
    {                                                                                               \
        engine::Print("\nAssertion failed in " STRINGIFY_FILE_LINE(__FILE__, __LINE__) "\n");       \
        engine::PrintSubMessage("\'" #isFalse "\' is false");                                       \
        engine::PrintSubMessage(__VA_ARGS__);                                                       \
        engine::Print("\n");                                                                        \
        __debugbreak();                                                                             \
    }

#define ASSERT_SUCCEEDED(hr, ...)                                                                   \
    if (FAILED(hr))                                                                                 \
    {                                                                                               \
        engine::Print("\nHRESULT failed in " STRINGIFY_FILE_LINE(__FILE__, __LINE__) "\n");         \
        engine::PrintSubMessage("hr = 0x%08X", hr);                                                 \
        engine::PrintSubMessage(__VA_ARGS__);                                                       \
        engine::Print("\n");                                                                        \
        __debugbreak();                                                                             \
    }

#define WARN_ONCE_IF(isTrue, ...)                                                                   \
    {                                                                                               \
        static bool s_TriggeredWarning = false;                                                     \
        if (!!(isTrue) && !s_TriggeredWarning)                                                      \
        {                                                                                           \
            s_TriggeredWarning = true;                                                              \
            engine::Print("\nWarning issued in " STRINGIFY_FILE_LINE(__FILE__, __LINE__) "\n");     \
            engine::PrintSubMessage("\'" #isTrue "\' is true");                                     \
            engine::PrintSubMessage(__VA_ARGS__);                                                   \
            engine::Print("\n");                                                                    \
        }                                                                                           \
    }

#define WARN_ONCE_IF_NOT(isTrue, ...) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#define ERROR(...)                                                                                  \
    engine::Print("\nError reported in " STRINGIFY_FILE_LINE(__FILE__, __LINE__) "\n");             \
    engine::PrintSubMessage(__VA_ARGS__);                                                           \
    engine::Print("\n");

#define DEBUGPRINT(msg, ...) GE::Printf(msg "\n", ##__VA_ARGS__);

#endif

#define HALT(...) ERROR(__VA_ARGS__) __debugbreak();

#define BREAK_IF_FAILED(hr) \
    if (FAILED(hr))         \
        __debugbreak()


