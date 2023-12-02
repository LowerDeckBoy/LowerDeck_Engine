#include "../../D3D/D3D12Viewport.hpp"
#include "../../D3D/D3D12Descriptor.hpp"
#include "../../D3D/D3D12RootSignature.hpp"
#include "../../D3D/D3D12PipelineState.hpp"
#include "../../D3D/D3D12DepthBuffer.hpp"

#include "LightPass.hpp"
#include "../../D3D/D3D12Utility.hpp"
#include "../../Utility/Utility.hpp"
#include "../../D3D/D3D12Device.hpp"
#include "../../D3D/D3D12Context.hpp"
#include "../../Graphics/TextureUtility.hpp"


LightPass::LightPass(D3D::D3D12Viewport* pViewport, D3D::D3D12DepthBuffer* pSceneDepth)
{
	m_SceneDepth = pSceneDepth;
	CreateTargets(pViewport);

	// Root Signature
	{
		D3D12_ROOT_SIGNATURE_FLAGS rootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED };
		std::vector<CD3DX12_ROOT_PARAMETER1> parameters(4);
		// Camera buffer
		parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE);
		// Texture indices: gbuffers, 
		parameters.at(1).InitAsConstants(5, 1, 0);
		// Texture indices: IBL
		parameters.at(2).InitAsConstants(4, 2, 0);
		// Lights data
		parameters.at(3).InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(2);
		samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		samplers.at(1) = D3D::Utility::CreateStaticSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		m_RootSignature.Create(parameters, samplers, rootFlags, L"Deferred Light Pass Root Signature");
	}

	// PSO
	{
		auto* builder{ new D3D::D3D12GraphicsPipelineStateBuilder() };
		auto layout{ D3D::Utility::GetScreenOutputInputLayout() };

		builder->SetInputLayout(layout);
		builder->SetVertexShader("Shaders/Deferred/Deferred.hlsl", L"VSmain");
		builder->SetPixelShader("Shaders/Deferred/Deferred.hlsl", L"PSmain");
		builder->SetEnableDepth(false);

		builder->Create(m_PSO, m_RootSignature.Get(), L"Deferred Light Pass PSO");
		builder->Reset();
	}
	
}

LightPass::~LightPass()
{
	Release();
}

void LightPass::BeginPass()
{
	D3D::TransitResource(m_OutputResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D::TransitResource(m_SceneDepth->Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

	auto depthHandle{ m_SceneDepth->DSV().GetCPU() };
	auto rtvHandle{ m_OutputRTVDesc.GetCPU() };
	D3D::g_CommandList.Get()->ClearRenderTargetView(m_OutputRTVDesc.GetCPU(), ClearColor.data(), 0, nullptr);
	D3D::g_CommandList.Get()->OMSetRenderTargets(1, &rtvHandle, TRUE, &depthHandle);

	D3D::SetPSO(m_PSO);
	D3D::SetRootSignature(m_RootSignature);
}

void LightPass::EndPass()
{
	D3D::TransitResource(m_OutputResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void LightPass::DrawData(const std::array<uint32_t, 5>& GBufferIndices, const std::array<uint32_t, 4>& IBLIndices)
{
	//D3D::g_CommandList.Get()->SetGraphicsRootConstantBufferView(0, m_cbCamera->GetBuffer()->GetGPUVirtualAddress());
	D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstants(1, sizeof(GBufferIndices) / sizeof(int32_t), GBufferIndices.data(), 0);
	D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstants(2, sizeof(IBLIndices) / sizeof(int32_t), IBLIndices.data(), 0);


}

void LightPass::OnResize(D3D::D3D12Viewport* pViewport)
{
	CreateTargets(pViewport);
}

void LightPass::Release()
{
	SAFE_RELEASE(m_OutputResource);
}

void LightPass::CreateTargets(D3D::D3D12Viewport* pViewport)
{
	if (m_OutputResource.Get())
		m_OutputResource.Reset();

	D3D12_RESOURCE_DESC outputDesc{};
	outputDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	outputDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	outputDesc.MipLevels = 1;
	outputDesc.DepthOrArraySize = 1;
	outputDesc.Width = static_cast<uint64_t>(pViewport->Viewport().Width);
	outputDesc.Height = static_cast<uint32_t>(pViewport->Viewport().Height);
	outputDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	outputDesc.SampleDesc = { 1, 0 };

	D3D12_CLEAR_VALUE clear{};
	clear.Color[0] = ClearColor.at(0);
	clear.Color[1] = ClearColor.at(1);
	clear.Color[2] = ClearColor.at(2);
	clear.Color[3] = ClearColor.at(3);
	clear.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
		&D3D::Utility::HeapDefault,
		D3D12_HEAP_FLAG_NONE,
		&outputDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clear,
		IID_PPV_ARGS(m_OutputResource.ReleaseAndGetAddressOf())));
	m_OutputResource.Get()->SetName(L"Deferred Light Pass Resource");

	D3D::D3D12Context::GetDeferredHeap()->Allocate(m_OutputRTVDesc);

	D3D::g_Device.Get()->CreateRenderTargetView(m_OutputResource.Get(), &rtvDesc, m_OutputRTVDesc.GetCPU());

	TextureUtility::CreateSRV(m_OutputResource.GetAddressOf(), m_OutputDescriptor, 1);
}
