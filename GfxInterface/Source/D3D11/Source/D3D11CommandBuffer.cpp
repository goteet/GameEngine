#include "D3D11CommandBuffer.h"

namespace GFXI
{
    std::vector<CommandBufferD3D11*> CommandBufferD3D11::mUnusedCommandQueues;

    CommandBufferD3D11::CommandBufferD3D11(ID3D11CommandList* commandList)
        : mD3D11CommandList(commandList)
    {

    }

    CommandBufferD3D11::~CommandBufferD3D11()
    {
        mD3D11CommandList.Reset();
    }
    void CommandBufferD3D11::Release()
    {
        mD3D11CommandList.Reset();
        {
            //TODO: multi-thread-support.
            mUnusedCommandQueues.push_back(this);
        }
    }

    void CommandBufferD3D11::FreeAllUnusedCommandQueues()
    {
        std::vector<CommandBufferD3D11*> pendingDeletedCommandQueues;

        {
            //TODO:multi-thread-support.
            pendingDeletedCommandQueues.swap(mUnusedCommandQueues);
        }

        for (auto& queue : pendingDeletedCommandQueues)
        {
            delete queue;
        }
    }

    CommandBufferD3D11* CommandBufferD3D11::GetOrCreateNewCommandQueue(ID3D11CommandList* commandList)
    {
        //TODO: multi-thread-support.
        CommandBufferD3D11* newQueue = nullptr;
        if (mUnusedCommandQueues.size() > 0)
        {
            newQueue = mUnusedCommandQueues.back();
            newQueue->mD3D11CommandList = commandList;
            mUnusedCommandQueues.pop_back();
        }
        else
        {
            newQueue = new CommandBufferD3D11(commandList);
        }
        return newQueue;
    }

    void CommandBufferD3D11::OnExecute(ID3D11DeviceContext* immediateContext, bool bRestoreToDefaultState)
    {
        immediateContext->ExecuteCommandList(mD3D11CommandList.Get(), !bRestoreToDefaultState);
    }
}
