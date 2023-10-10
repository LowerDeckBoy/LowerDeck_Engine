#include "../../D3D/D3D12Descriptor.hpp"
#include "../../D3D/D3D12RootSignature.hpp"
#include "../../D3D/D3D12GraphicsPipelineState.hpp"
#include "../../D3D/D3D12Viewport.hpp"
#include "../../D3D/D3D12DepthBuffer.hpp"
#include "GBufferPass.hpp"
#include "../../D3D/D3D12Command.hpp"
#include "../../D3D/D3D12Context.hpp"


GBufferPass::GBufferPass(D3D::D3D12Viewport* pViewport, D3D::D3D12DepthBuffer* pSceneDepth, std::shared_ptr<gfx::ShaderManager> pShaderManager)
{
	m_SceneDepth = pSceneDepth;

	Initialize(pViewport, pShaderManager);
}

GBufferPass::~GBufferPass()
{
	Release();
}

void GBufferPass::BeginPass()
{
	// Render Targets to Render State and Clear Targets
	for (size_t i = 0; i < RenderTargetsCount; i++)
	{
		D3D::TransitResource(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D::g_CommandList.Get()->ClearRenderTargetView(m_RenderTargetDescs.at(i), m_ClearColor.data(), 0, nullptr);
	}

	auto depthHandle{ m_SceneDepth->DSV().GetCPU() };
	D3D::g_CommandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);

	D3D::g_CommandList.Get()->OMSetRenderTargets(RenderTargetsCount, m_RenderTargetDescs.data(), false, &depthHandle);

	D3D::SetPSO(m_PSO);
	D3D::SetRootSignature(m_RootSignature);
}

void GBufferPass::EndPass()
{
	// Render Targets to Generic Read
	for (auto& renderTarget : m_RenderTargets)
		D3D::TransitResource(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void GBufferPass::OnResize(D3D::D3D12Viewport* pViewport)
{
	CreateRenderTargets(pViewport);
}

void GBufferPass::Release()
{
	m_RootSignature.Release();
	m_PSO.Release();

	for (auto& target : m_RenderTargets)
		SAFE_RELEASE(target);
}

void GBufferPass::Initialize(D3D::D3D12Viewport* pViewport, std::shared_ptr<gfx::ShaderManager> pShaderManager)
{
	D3D12_ROOT_SIGNATURE_FLAGS rootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED };

	std::vector<CD3DX12_ROOT_PARAMETER1> parameters(2);
	// Per Object Matrices
	parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	// Material indices + material data
	parameters.at(1).InitAsConstants(20, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	// Camera Buffer
	//parameters.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
	samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
	m_RootSignature.Create(parameters, samplers, rootFlags, L"Deferred GBuffer Root Signature");

	auto* builder{ new D3D::D3D12GraphicsPipelineStateBuilder(pShaderManager) };

	auto layout{ D3D::Utility::GetModelInputLayout() };
	builder->SetInputLayout(layout);
	builder->SetVertexShader("Shaders/Deferred/GBuffer.hlsl", L"VSmain");
	builder->SetPixelShader("Shaders/Deferred/GBuffer.hlsl", L"PSmain");
	builder->SetRenderTargetFormats(m_RenderTargetFormats);

	builder->Create(m_PSO, m_RootSignature.Get(), L"GBuffer PSO");
	builder->Reset();


	CreateRenderTargets(pViewport);
}

void GBufferPass::CreateRenderTargets(D3D::D3D12Viewport* pViewport)
{
	if (m_RenderTargets.at(0).Get())
	{
		for (auto& target : m_RenderTargets)
			SAFE_RELEASE(target);
	}

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Width = static_cast<uint64_t>(pViewport->Viewport().Width);
	desc.Height = static_cast<uint32_t>(pViewport->Viewport().Height);
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	desc.SampleDesc = { 1, 0 };

	D3D12_CLEAR_VALUE clear{};
	clear.Color[0] = m_ClearColor.at(0);
	clear.Color[1] = m_ClearColor.at(1);
	clear.Color[2] = m_ClearColor.at(2);
	clear.Color[3] = m_ClearColor.at(3);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3D::D3D12Context::GetDeferredHeap()->GetCPUHandle());
	const auto heapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
	for (uint32_t i = 0; i < RenderTargetsCount; i++)
	{
		desc.Format = m_RenderTargetFormats.at(i);
		clear.Format = m_RenderTargetFormats.at(i);
		ThrowIfFailed(D3D::g_Device.Get()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clear,
			IID_PPV_ARGS(m_RenderTargets.at(i).ReleaseAndGetAddressOf())));

		const std::wstring wname{ L"Deferred Render Target #" + std::to_wstring(i) };
		m_RenderTargets.at(i).Get()->SetName(wname.c_str());

		m_RenderTargetDescs.at(i) = rtvHandle;
		D3D::g_Device.Get()->CreateRenderTargetView(m_RenderTargets.at(i).Get(), &rtvDesc, m_RenderTargetDescs.at(i));
		rtvHandle.Offset(1, 32);

		// SRVs
		D3D::D3D12Context::GetMainHeap()->Allocate(m_SRVDescs.at(i));
		D3D::g_Device.Get()->CreateShaderResourceView(m_RenderTargets.at(i).Get(), &srvDesc, m_SRVDescs.at(i).GetCPU());
	}
}
