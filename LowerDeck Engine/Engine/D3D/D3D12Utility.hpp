#pragma once
#include <d3dx12.h>
#include <array>
#include <cstdint>
#include <wrl/client.h>

enum class ShaderType : uint8_t;

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

namespace D3D
{
	// Number of total frames used in SwapChain.
	// aka Frames in Flight
	constexpr uint32_t FRAME_COUNT{ 3 };

	// ============================================= State helpers =============================================
	
	/// <summary>
	/// Helper function for building Root Signatures and PSOs.<br/>
	/// Part of D3D namespace to prevent collision with general utilities.
	/// </summary>
	class Utility
	{
	public:

		/// <summary>
		/// 
		/// </summary>
		/// <param name="ShaderRegister"></param>
		/// <param name="RegisterSpace"></param>
		/// <param name="Filter"></param>
		/// <param name="AddressMode"></param>
		/// <param name="ComparsionFunc"></param>
		/// <param name="Visibility"></param>
		/// <returns> D3D12_STATIC_SAMPLER_DESC </returns>
		static D3D12_STATIC_SAMPLER_DESC CreateStaticSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

		// ============================================= INPUT LAYOUTS =============================================
		/// <summary> 3D Model layout. </summary>
		/// <returns>
		/// Array of:
		/// XMFLOAT3 Position, 
		/// XMFLOAT2 TexCoord, 
		/// XMFLOAT3 Normal,
		/// XMFLOAT3 Tangent, 
		/// XMFLOAT3 Bitangent
		/// </returns>
		static std::array<D3D12_INPUT_ELEMENT_DESC, 5> GetModelInputLayout();

		/// <summary>
		/// Plain skybox and Image Based Lighting usage.
		/// </summary>
		/// <returns>
		/// Array of:
		/// XMFLOAT3 Position,
		/// XMFLOAT3 TexCoord
		/// </returns>
		static std::array<D3D12_INPUT_ELEMENT_DESC, 2> GetSkyInputLayout();

		/// <summary>
		/// Used for deferred rendering screen output (aka screen quad).
		/// </summary>
		/// <returns>
		/// Array of:
		/// XMFLOAT3 Position,
		/// XMFLOAT2 TexCoord
		///</returns>
		static std::array<D3D12_INPUT_ELEMENT_DESC, 2> GetScreenOutputInputLayout();

	};
	
}

namespace gfx
{
	/// <summary>
	/// Converts ShaderType enum value to LPCWSTR for shader creation usage.
	/// </summary>
	/// <param name="TypeOf"></param>
	/// <returns></returns>
	LPCWSTR ShaderTypeToName(ShaderType TypeOf);
}
