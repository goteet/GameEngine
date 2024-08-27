#pragma once
#include <vector>
#include "D3D11Include.h"
#include "GfxInterface.h"


namespace GFXI
{
    using Microsoft::WRL::ComPtr;

    struct CommandQueueD3D11 : public CommandQueue
    {;
        ~CommandQueueD3D11();
        virtual void Release() override;

        static void FreeAllUnusedCommandQueues();
        static CommandQueueD3D11* GetOrCreateNewCommandQueue(ID3D11CommandList* commandList);
        
        void OnExecute(ID3D11DeviceContext* immediateContext, bool bRestoreToDefaultState);

    private:
        static std::vector<CommandQueueD3D11*> mUnusedCommandQueues;
        CommandQueueD3D11(ID3D11CommandList* commandList);

        ComPtr<ID3D11CommandList> mD3D11CommandList;
    };
}
