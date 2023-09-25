#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include <span>

namespace D3D
{
	/// <summary>
	/// 
	/// </summary>
	class D3D12RootSignature
	{
	public:
		D3D12RootSignature() = default;
		/// <summary>
		/// Creates Root Signature upon construction.
		/// </summary>
		/// <param name="Parameters"> Either std::vector or std::array. </param>
		/// <param name="Samplers"> Either std::vector or std::array. </param>
		/// <param name="RootFlags"> Recommanded to use minimal access. </param>
		/// <param name="DebugName"> Optional. </param>
		D3D12RootSignature(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters, const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers, const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags, LPCWSTR DebugName = L"");
		/// <summary>
		/// Calls Release() if it haven't been called manually.
		/// </summary>
		~D3D12RootSignature();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="Parameters"> Either std::vector or std::array. </param>
		/// <param name="Samplers"> Either std::vector or std::array. </param>
		/// <param name="RootFlags"> Recommanded to use minimal access. </param>
		void Create(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters, const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers, const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags, LPCWSTR DebugName = L"");

		/// <summary> Getter. </summary>
		/// <returns> Underlaying ComPtr. </returns>
		[[nodiscard]] ID3D12RootSignature* Get() { return m_RootSignature.Get(); }

		/// <summary>
		/// As the name implies.
		/// </summary>
		void Release();

	private:
		// TODO:
		// Move later into D3D12Utility.hpp for later DXR usage
		/// <summary>
		/// Serializes versioned Root Signature.<br/>
		/// Version defaults to <b>D3D_ROOT_SIGNATURE_VERSION_1_1</b>.
		/// </summary>
		/// <param name="Desc"></param>
		/// <param name="DebugName"> Optional. </param>
		void SerializeAndCreate(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, LPCWSTR DebugName = L"");
		
		/// <summary> Actual RootSignature object. </summary>
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	};
}
