#include "GfxD3D12.h"

#include "ErrorReport.h"

#include <winreg.h>

namespace engine
{
	namespace gfx
	{
		GfxD3D12CommandAllocatorPool::GfxD3D12CommandAllocatorPool(GfxD3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
			: GfxD3D12DeviceChild(device)
			, mCommandListType(type) { }

		GfxD3D12CommandAllocatorPool::~GfxD3D12CommandAllocatorPool()
		{
			for (ID3D12CommandAllocator* commandAllocator : mAllocatorPool)
				commandAllocator->Release();

			mAllocatorPool.clear();
		}

		ID3D12CommandAllocator* GfxD3D12CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue)
		{
			GfxLockGuard lock(mAllocatorMutex);

			ID3D12CommandAllocator* pAllocator = nullptr;

			if (!mReadyAllocators.empty())
			{
				std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = mReadyAllocators.front();

				if (AllocatorPair.first <= completedFenceValue)
				{
					pAllocator = AllocatorPair.second;
					ASSERT_SUCCEEDED(pAllocator->Reset());
					mReadyAllocators.pop();
				}
			}

			if (pAllocator == nullptr)
			{
				ASSERT_SUCCEEDED(mDevice->GetDevice()->CreateCommandAllocator(mCommandListType, IID_PPV_ARGS(&pAllocator)));
				wchar_t allocatorName[32];
				swprintf(allocatorName, 32, L"CommandAllocator %zu", mAllocatorPool.size());
				pAllocator->SetName(allocatorName);
				mAllocatorPool.push_back(pAllocator);
			}

			return pAllocator;
		}

		void GfxD3D12CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator)
		{
			GfxLockGuard lock(mAllocatorMutex);

			mReadyAllocators.push(std::make_pair(fenceValue, commandAllocator));
		}

		GfxD3D12CommandQueue::GfxD3D12CommandQueue(GfxD3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
			: GfxD3D12DeviceChild(device)
			, mCommandAllocatorPool(device, type)
			, mQueueType(type)
			, mFence(nullptr)
			, mNextFenceValue()
			, mLastCompletedFenceValue()
			, mFenceEventHandle(nullptr)
		{ }

		GfxD3D12CommandQueue::~GfxD3D12CommandQueue()
		{
			if (mCommandQueue != nullptr)
			{
				CloseHandle(mFenceEventHandle);
				mFence->Release();
				mFence = nullptr;
				mCommandQueue->Release();
				mCommandQueue = nullptr;
			}
		}

		uint64_t GfxD3D12CommandQueue::IncrementFence()
		{
			GfxLockGuard lock(mFenceMutex);

			mCommandQueue->Signal(mFence, mNextFenceValue);
			return mNextFenceValue++;
		}

		bool GfxD3D12CommandQueue::IsFenceComplete(uint64_t fenceValue)
		{
			if (fenceValue > mLastCompletedFenceValue)
			{
				mLastCompletedFenceValue = std::max(mLastCompletedFenceValue, mFence->GetCompletedValue());
			}
			return fenceValue <= mLastCompletedFenceValue;
		}

		void GfxD3D12CommandQueue::StallForCommandQueue(GfxD3D12CommandQueue& commandQueue)
		{
			ASSERT(commandQueue.mNextFenceValue > 0);

			mCommandQueue->Wait(commandQueue.mFence, commandQueue.mNextFenceValue-1);
		}

		void GfxD3D12CommandQueue::WaitForFence(uint64_t fenceValue)
		{
			if (IsFenceComplete(fenceValue))
				return;

			{
				GfxLockGuard lock(mEventMutex);

				mFence->SetEventOnCompletion(fenceValue, mFenceEventHandle);
				WaitForSingleObject(mFenceEventHandle, INFINITE);
				mLastCompletedFenceValue = fenceValue;
			}
		}

		void GfxD3D12CommandQueue::WaitForIdle()
		{
			WaitForFence( IncrementFence() );
		}

		uint64_t GfxD3D12CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
		{
			GfxLockGuard lock(mFenceMutex);

			ASSERT_SUCCEEDED(((ID3D12GraphicsCommandList*)commandList)->Close());

			mCommandQueue->ExecuteCommandLists(1, &commandList);
			mCommandQueue->Signal(mFence, mNextFenceValue);
			return mNextFenceValue++;
		}

		ID3D12CommandAllocator* GfxD3D12CommandQueue::RequestAllocator()
		{
			uint64_t completedFenceValue = mFence->GetCompletedValue();
			return mCommandAllocatorPool.RequestAllocator(completedFenceValue);
		}

		void GfxD3D12CommandQueue::DiscardAllocator(uint64_t fenceValueForReset, ID3D12CommandAllocator* commandAllocator)
		{
			mCommandAllocatorPool.DiscardAllocator(fenceValueForReset, commandAllocator);
		}

		GfxD3D12Device::GfxD3D12Device()
		{
			/**
			 * init D3D12 adapter, device and swapchain.
			 */

			ComPtr<ID3D12Device> pDevice;

			uint32_t useDebugLayers = 0;
#if _DEBUG
			useDebugLayers = 1;
#endif

			DWORD dxgiFactoryFlags = 0;

			if (useDebugLayers)
			{
				ComPtr<ID3D12Debug> debugInterface;
				if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
				{
					debugInterface->EnableDebugLayer();

					uint32_t useGPUBasedValidation = 0;
#if _DEBUG
					useGPUBasedValidation = 1;
#endif
					if (useGPUBasedValidation)
					{
						ComPtr<ID3D12Debug1> debugInterface1;
						if (SUCCEEDED((debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)))))
						{
							debugInterface1->SetEnableGPUBasedValidation(true);
						}
					}
				}

#if _DEBUG
				ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(pDxgiInfoQueue.GetAddressOf()))))
				{
					dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

					pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
					pDxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

					DXGI_INFO_QUEUE_MESSAGE_ID hide[] = { 80 };
					DXGI_INFO_QUEUE_FILTER filter = {};
					filter.DenyList.NumIDs = _countof(hide);
					filter.DenyList.pIDList = hide;
					pDxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
				}
#endif
			}

			ComPtr<IDXGIFactory6> pDxgiFactory;
			ASSERT_SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pDxgiFactory)));

			ComPtr<IDXGIAdapter1> pAdapter;

			D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);

			SIZE_T maxSize = 0;
			for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(i, &pAdapter); ++i)
			{
				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				// Is a software adapter?
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				// Can create a D3D12 device?
				if (FAILED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice))))
					continue;

				// By default, search for the adapter with the most memory because that's usually the dGPU.
				if (desc.DedicatedVideoMemory < maxSize)
					continue;

				maxSize = desc.DedicatedVideoMemory;

				if (mDevice != nullptr)
					mDevice->Release();

				mDevice = pDevice.Detach();

				Printf(L"Selected GPU:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
			}

			if (mDevice == nullptr)
			{
				Print("Failed to find a hardware adapter.  Falling back to WARP.\n");

				ASSERT_SUCCEEDED(pDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));
				ASSERT_SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)));
				mDevice = pDevice.Detach();
			}
#ifndef RELEASE
			else
			{
				bool bDeveloperModeEnabled = false;

				// Look in the Windows Registry to determine if Developer Mode is enabled
				HKEY hKey;
				LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
				if (result == ERROR_SUCCESS)
				{
					DWORD keyValue, keySize = sizeof(DWORD);
					result = RegQueryValueEx(hKey, L"AllowDevelopmentWithoutDevLicense", 0, NULL, (byte*)&keyValue, &keySize);
					if (result == ERROR_SUCCESS && keyValue == 1)
						bDeveloperModeEnabled = true;
					RegCloseKey(hKey);
				}

				WARN_ONCE_IF_NOT(bDeveloperModeEnabled, "Enable Developer Mode on Windows 10 to get consistent profiling results");

				// Prevent the GPU from overclocking or underclocking to get consistent timings
				if (bDeveloperModeEnabled)
					mDevice->SetStablePowerState(TRUE);
			}
#endif

#if _DEBUG
			ID3D12InfoQueue* pInfoQueue = nullptr;
			if (SUCCEEDED(mDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
			{
				D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

				D3D12_MESSAGE_ID DenyIds[] =
				{
					D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
					D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,
					D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

					(D3D12_MESSAGE_ID)1008,
				};

				D3D12_INFO_QUEUE_FILTER NewFilter = {};
				NewFilter.DenyList.NumSeverities = _countof(Severities);
				NewFilter.DenyList.pSeverityList = Severities;
				NewFilter.DenyList.NumIDs = _countof(DenyIds);
				NewFilter.DenyList.pIDList = DenyIds;

				pInfoQueue->PushStorageFilter(&NewFilter);
				pInfoQueue->Release();
			}
#endif

		}

		GfxD3D12Device::~GfxD3D12Device()
		{
		}

		GE::GfxDefaultVertexBuffer* GfxD3D12Device::CreateDefaultVertexBuffer(unsigned int vertexCount)
		{
			return nullptr;
		}

		GE::GfxDefaultIndexBuffer* GfxD3D12Device::CreateDefaultIndexBuffer(unsigned int indexCount)
		{
			return nullptr;
		}

		GfxD3D12VertexBuffer* GfxD3D12Device::CreateVertexBuffer()
		{
			return nullptr;
		}

		GfxD3D12IndexBuffer* GfxD3D12Device::CreateIndexBuffer()
		{
			return nullptr;
		}

		GfxD3D12ConstantBuffer* GfxD3D12Device::CreateConstantBuffer()
		{
			return nullptr;
		}

	}
}