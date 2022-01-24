#include <Foundation/Base/MemoryHelper.h>
#include <Foundation/Base/ScopeHelper.h>
#include "GEInclude.h"

#include "Core/GameEngine.h"

namespace GE
{
    engine::GameEngine* sGlobalInstance = nullptr;

    bool GEObject::IsSameType(GEObject* other) const { return GetClassUID() == other->GetClassUID(); }

    ClassUID GameEngineAPI GenerateClassUID()
    {
        static ClassUID sNextClassID = 0;
        return ++sNextClassID;
    }

    GameEngine::InitializeResult GameEngine::Initialize(const GameEngine::CreationConfig& config, GameEngine*& outInstance)
    {
        engine::GameEngine*& pEngine = sGlobalInstance;

        if (pEngine == nullptr)
        {
            auto failGuard = base::make_scope_guard([&]
                {
                    SafeDelete(pEngine);
                });

            pEngine = new engine::GameEngine();

            if (pEngine->InitializeMe(config))
            {
                failGuard.dismiss();
                outInstance = pEngine;
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
