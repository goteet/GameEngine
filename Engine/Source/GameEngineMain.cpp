#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Base/ScopeHelper.h>
#include "GEInclude.h"

namespace GE
{
    GameEngine* sGlobalInstance = nullptr;

    bool GEObject::IsSameType(GEObject* other) const { return GetClassUID() == other->GetClassUID(); }

    ClassUID GameEngineAPI GenerateClassUID()
    {
        static ClassUID sNextClassID = 0;
        return ++sNextClassID;
    }

    GameEngine::InitializeResult GameEngine::Initialize(const GameEngine::CreationConfig& config, GameEngine*& outInstance)
    {
        //::Engine*& pEngine = ::GameEngine::Instance;

        GameEngine* pEngine = sGlobalInstance;

        if (pEngine != nullptr)
        {
            auto failGuard = base::make_scope_guard([&]
                {
                    SafeDelete(pEngine);
                });

            //pEngine = new ::Engine();

            //if (pEngine->Initialize(config, outInstance))
            if (false)
            {
                failGuard.dismiss();
                return InitializeResult::Success;
            }

            return InitializeResult::Error;
        }
        else
        {
            return InitializeResult::AlreadyInitialized;
        }
    }

    void GameEngine::Uninitialize()
    {
        SafeDelete(sGlobalInstance);
    }

    GameEngine* GameEngine::GetInstance()
    {
        return sGlobalInstance;
    }

    bool GameEngine::IsInitialized()
    {
        return GetInstance() != nullptr;
    }
}
