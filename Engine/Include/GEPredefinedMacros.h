#pragma once

// clang-format off
#ifdef _MSC_VER
    #ifdef GAME_ENGINE_EXPORT
        #define GameEngineAPI __declspec(dllexport)
    #else
        #define GameEngineAPI __declspec(dllimport)
    #endif
#else
    #ifdef GAME_ENGINE_EXPORT
        #define GameEngineAPI __attribute__ ((visibility ("default"))) 
    #else
        #define GameEngineAPI 
    #endif
#endif
// clang-format on

namespace GE
{
    using ClassUID = unsigned int;

    struct GameEngineAPI GEObject
    {
        virtual ClassUID GetClassUID() const = 0;
        bool IsSameType(GEObject* other) const;

    protected:
        virtual ~GEObject() = default;
        GEObject() = default;
        GEObject(const GEObject&) = delete;
        GEObject& operator=(const GEObject&) = delete;
    };

    ClassUID GameEngineAPI GenerateClassUID();

    template<class InterfaceType> bool Is(GEObject* obj)
    {
        return obj->GetClassUID() == InterfaceType::GetStaticClassUID();
    }

#define DefineRTTI                                                          \
    public:                                                                 \
        static GE::ClassUID GetStaticClassUID()                             \
        {                                                                   \
            static GE::ClassUID sStaticClassUID = GE::GenerateClassUID();   \
            return sStaticClassUID;                                         \
        }                                                                   \
        virtual GE::ClassUID GetClassUID() const override { return GetStaticClassUID(); }

}    // namespace GE
