#pragma once

#include "PreIncludeFiles.h"

#include <mutex>
#include <queue>
#include <vector>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace engine
{
	namespace gfx
	{
		template<typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;

		using GfxLockGuard = std::lock_guard<std::mutex>;

		struct GfxD3D12Device;

		struct GfxD3D12DeviceChild
		{
			explicit GfxD3D12DeviceChild(GfxD3D12Device* device = nullptr) : mDevice(device) { }

	 	protected:
			GfxD3D12Device* mDevice;
		};

		struct GfxD3D12Resource
		{
			GfxD3D12Resource()
				: mResource(nullptr)
				, mUsageState(D3D12_RESOURCE_STATE_COMMON)
				, mTransitionState((D3D12_RESOURCE_STATES)-1)
				, mGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
			{}

			virtual ~GfxD3D12Resource() = 0;

			FORCEINLINE ID3D12Resource* GetResource() { return mResource.Get(); }
			FORCEINLINE const ID3D12Resource* GetResource() const { return mResource.Get(); }
			FORCEINLINE D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddr() const { return mGPUVirtualAddress; }

			FORCEINLINE D3D12_RESOURCE_STATES GetUsageState() { return mUsageState; }
			FORCEINLINE D3D12_RESOURCE_STATES GetTransitionState() { return mTransitionState; }

			ComPtr<ID3D12Resource> mResource;
			D3D12_RESOURCE_STATES mUsageState;
			D3D12_RESOURCE_STATES mTransitionState;
			D3D12_GPU_VIRTUAL_ADDRESS mGPUVirtualAddress;
		};

		struct GfxD3D12Buffer : public GfxD3D12Resource { };

		struct GfxD3D12ConstantBuffer : public GfxD3D12Buffer { };

		struct GfxD3D12VertexBuffer : public GfxD3D12Buffer { };

		struct GfxD3D12IndexBuffer : public GfxD3D12Buffer { };

		struct GfxD3D12CommandAllocatorPool : public GfxD3D12DeviceChild
		{
			GfxD3D12CommandAllocatorPool(GfxD3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
			~GfxD3D12CommandAllocatorPool();

			ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);
			void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator);

		private:
			const D3D12_COMMAND_LIST_TYPE mCommandListType;
			std::vector<ID3D12CommandAllocator*> mAllocatorPool;
			std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> mReadyAllocators;
			std::mutex mAllocatorMutex;
		};

		struct GfxD3D12CommandQueue : public GfxD3D12DeviceChild
		{
			GfxD3D12CommandQueue(GfxD3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
			~GfxD3D12CommandQueue();

			uint64_t IncrementFence();
			bool IsFenceComplete(uint64_t fenceValue);
			void StallForCommandQueue(GfxD3D12CommandQueue& commandQueue);
			void WaitForFence(uint64_t fenceValue);
			void WaitForIdle();

			FORCEINLINE ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue; }
			FORCEINLINE uint64_t GetNextFenceValue() const { return mNextFenceValue; }

		private:
			uint64_t ExecuteCommandList(ID3D12CommandList* commandList);
			ID3D12CommandAllocator* RequestAllocator();
			void DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* commandAllocator);

			GfxD3D12CommandAllocatorPool mCommandAllocatorPool;
			ID3D12CommandQueue* mCommandQueue;
			D3D12_COMMAND_LIST_TYPE mQueueType;

			std::mutex mFenceMutex;
			std::mutex mEventMutex;

			ID3D12Fence* mFence;
			uint64_t mNextFenceValue;
			uint64_t mLastCompletedFenceValue;
			HANDLE mFenceEventHandle;
		};

		struct GfxD3D12Device : GE::GfxDevice
		{
			DefineRTTI;

			GfxD3D12Device();
			~GfxD3D12Device();

			virtual GE::GfxDefaultVertexBuffer* CreateDefaultVertexBuffer(unsigned int vertexCount) override;
			virtual GE::GfxDefaultIndexBuffer* CreateDefaultIndexBuffer(unsigned int indexCount) override;

			GfxD3D12VertexBuffer* CreateVertexBuffer();
			GfxD3D12IndexBuffer* CreateIndexBuffer();
			GfxD3D12ConstantBuffer* CreateConstantBuffer();

			FORCEINLINE ID3D12Device* GetDevice() { return mDevice.Get(); }

			ComPtr<ID3D12Device> mDevice;

			ComPtr<ID3D12RootSignature> mRootSignature;
			ComPtr<ID3D12DescriptorHeap> mRtvHeap;
			ComPtr<ID3D12DescriptorHeap> mDsvHeap;
			ComPtr<ID3D12DescriptorHeap> mCbvSrvHeap;
			ComPtr<ID3D12DescriptorHeap> mSamplerHeap;
		};

	} // namespace gfx
} // namespace engine