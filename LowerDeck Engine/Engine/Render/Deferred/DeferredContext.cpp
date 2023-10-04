#include "../Model/Model.hpp"
#include "../Camera.hpp"
#include "DeferredContext.hpp"
#include "../../D3D/D3D12Context.hpp"
#include <ImGui/imgui.h>


DeferredContext::DeferredContext(std::shared_ptr<D3D::D3D12Context> pD3DContext, std::shared_ptr<gfx::ShaderManager> pShaderManager, D3D::D3D12DepthBuffer* DepthBuffer)
	: ScreenOutput()
{
	m_D3DContext	= pD3DContext;
	m_ShaderManager = pShaderManager;
	m_SceneDepth	= DepthBuffer;

	Initialize();
}

DeferredContext::~DeferredContext()
{
	Release();
}

void DeferredContext::Initialize()
{
	m_DeferredHeap = std::make_unique<D3D::D3D12DescriptorHeap>(D3D::HeapUsage::eRTV, RenderTargetsCount, L"Deferred Render Target Heap");

	ScreenOutput::Create();

	CreateRenderTargets();

	CreateRootSignatures();
	CreatePipelines();
}

void DeferredContext::OnResize()
{
	CreateRenderTargets();
}

void DeferredContext::Release()
{
	ScreenOutput::Release();

	m_D3DContext = nullptr;

	m_GBufferRootSignature.Release();
	m_GBufferPSO.Release();

	m_OutputRootSignature.Release();
	m_OutputPSO.Release();

	m_DeferredHeap->Release();

	for (auto& target : m_RenderTargets)
		SAFE_RELEASE(target);
}

void DeferredContext::PassGBuffer(Camera* pCamera, const std::vector<std::unique_ptr<Model>>& Models)
{
	// Render Targets to Render State
	// Clear Targets
	for (size_t i = 0; i < RenderTargetsCount; i++)
	{
		D3D::TransitResource(m_RenderTargets.at(i).Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D::g_CommandList.Get()->ClearRenderTargetView(m_RenderDescs.at(i), m_ClearColor.data(), 0, nullptr);
	}
	
	D3D::SetPSO(m_GBufferPSO);
	D3D::SetRootSignature(m_GBufferRootSignature);
	auto depthHandle{ m_SceneDepth->DSV().GetCPU() };
	D3D::g_CommandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);

	D3D::g_CommandList.Get()->OMSetRenderTargets(RenderTargetsCount, m_RenderDescs.data(), false, &depthHandle);

	for (const auto& model : Models)
		model->Draw(pCamera);

	// Render Targets to Generic Read
	for (auto& renderTarget : m_RenderTargets)
		D3D::TransitResource(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void DeferredContext::CreateRenderTargets()
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
	desc.Width  = static_cast<uint64_t>(m_D3DContext->GetSceneViewport()->Viewport().Width);
	desc.Height = static_cast<uint32_t>(m_D3DContext->GetSceneViewport()->Viewport().Height);
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

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DeferredHeap->GetCPUHandle());
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

		m_RenderDescs.at(i) = rtvHandle;
		D3D::g_Device.Get()->CreateRenderTargetView(m_RenderTargets.at(i).Get(), &rtvDesc, m_RenderDescs.at(i));
		rtvHandle.Offset(1, 32);

		// SRVs
		D3D::D3D12Context::GetMainHeap()->Allocate(m_ShaderDescs.at(i));
		D3D::g_Device.Get()->CreateShaderResourceView(m_RenderTargets.at(i).Get(), &srvDesc, m_ShaderDescs.at(i).GetCPU());
	}
}

void DeferredContext::CreateRootSignatures()
{
	// GBuffer
	{
		D3D12_ROOT_SIGNATURE_FLAGS rootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |						D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | 
					D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED };
		//
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges(2);
		//ranges.at(0) = CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1024, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, OFFSET)

		std::vector<CD3DX12_ROOT_PARAMETER1> parameters(3);
		// Per Object Matrices
		parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// Camera Buffer
		parameters.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// Material indices + material data
		parameters.at(2).InitAsConstants(20, 2, 0, D3D12_SHADER_VISIBILITY_ALL);
		//parameters.at(2).InitAsConstants(4, 2, 0, D3D12_SHADER_VISIBILITY_ALL);
		// Material data
		//parameters.at(3).InitAsConstants(16, 3, 0, D3D12_SHADER_VISIBILITY_ALL);



		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
		samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		m_GBufferRootSignature.Create(parameters, samplers, rootFlags, L"Default Root Signature");
	}

	// Output
	{

	}

}

void DeferredContext::CreatePipelines()
{
	auto* builder{ new D3D::D3D12GraphicsPipelineStateBuilder(m_ShaderManager) };

	// GBuffer
	{
		auto layout{ D3D::Utility::GetModelInputLayout() };
		builder->SetVertexShader("Shaders/Deferred/GBuffer.hlsl", L"VSmain");
		builder->SetPixelShader("Shaders/Deferred/GBuffer.hlsl", L"PSmain");
		builder->SetInputLayout(layout);
		builder->SetRenderTargetFormats(m_RenderTargetFormats);

		builder->Create(m_GBufferPSO, m_GBufferRootSignature.Get(), L"GBuffer PSO");
	}

	// Deferred Output
	{

	}

	builder->Reset();
	delete builder;
}
