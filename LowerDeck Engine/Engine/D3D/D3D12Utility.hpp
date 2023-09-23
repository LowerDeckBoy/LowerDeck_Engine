#pragma once
#include <d3dx12.h>
#include <array>
#include <cstdint>
#include <wrl/client.h>

namespace D3D
{
	// Number of total frames used in SwapChain.
	// aka Frames in Flight
	constexpr uint32_t FRAME_COUNT{ 3 };
}

// Index of a frame currently processing by SwapChain
extern uint32_t FRAME_INDEX;

#ifndef SAFE_RELEASE
// Releasing ComPtr
#define SAFE_RELEASE(_ComPtr) { if (_ComPtr) { _ComPtr.Reset(); _ComPtr = nullptr; } }
#endif

#ifndef SAFE_DELETE
// Releasing IUnknown pointer that is NOT of a ComPtr type
#define SAFE_DELETE(_Ptr) { if (_Ptr) { _Ptr->Release(); _Ptr = nullptr; } }
#endif
