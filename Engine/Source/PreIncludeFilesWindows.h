#pragma once
// TODO: We should include platform header firstly, which system related APIs need this.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#include <d3d11_3.h>
#include <dxgi1_3.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#ifdef _DEBUG
	#include <dxgidebug.h>
#endif
#include <wrl/client.h>
