#include "D3D12RootSignature.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	D3D12RootSignature::D3D12RootSignature(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters, const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers, const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags, LPCWSTR DebugName)
	{
		Create(Parameters, Samplers, RootFlags, DebugName);
	}

	D3D12RootSignature::~D3D12RootSignature()
	{
		if (m_RootSignature.Get())
			Release();
	}

	void D3D12RootSignature::Create(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters, const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers, const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags, LPCWSTR DebugName)
	{
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
		rootDesc.Init_1_1(
			static_cast<uint32_t>(Parameters.size()), Parameters.data(),
			static_cast<uint32_t>(Samplers.size()), Samplers.data(),
			RootFlags);
	
		SerializeAndCreate(rootDesc, DebugName);
	}

	void D3D12RootSignature::Release()
	{
		SAFE_RELEASE(m_RootSignature);
	}

	void D3D12RootSignature::SerializeAndCreate(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, LPCWSTR DebugName)
	{
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(
			&Desc, D3D_ROOT_SIGNATURE_VERSION_1_0, signature.GetAddressOf(), error.GetAddressOf()));

		ThrowIfFailed(D3D::g_Device.Get()->CreateRootSignature(
		0, 
		signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.ReleaseAndGetAddressOf())));

		if (DebugName)
			m_RootSignature.Get()->SetName(DebugName);

		SAFE_DELETE(signature);
		SAFE_DELETE(error);
	}

}
