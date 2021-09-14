#pragma once
#include <Foundation/Math/Matrix.h>
#include <Foundation/Math/Rotation.h>
#include <Foundation/Math/Geometry.h>
#include "GEPredefinedMacros.h"
#include "GEInput.h"
#include "GERender.h"
#include "GEScene.h"


namespace GE
{
    struct GameEngineAPI GameEngine : public GEObject
    {
        struct CreationConfig
        {
            // Native rendering window handle
            //	eg. HWND, EGLWindowType, etc.
            void* NativeWindow;

            // Engine will change all relative resource-loading paths with this prefix,
            // turning them to absolute paths.
            const char* AbsoluteResourceFolderPath;
        };

        enum class InitializeResult { Success, AlreadyInitialized, Error };

        static InitializeResult Initialize(const CreationConfig& config, GameEngine* &outInstance);
        static void Uninitialize();
        static bool IsInitialized();
        static GameEngine* GetInstance();

        //engine interface
        virtual RenderSystem* GetRenderSystem() = 0;

        virtual Scene* CreateNewScene() = 0;

        virtual void Update(unsigned int deltaMillisec) = 0;
        virtual void OnMessage(const Message& message) = 0;
        virtual bool OnResize(unsigned int width, unsigned int height) = 0;

        virtual void Release() = 0;
    };
}