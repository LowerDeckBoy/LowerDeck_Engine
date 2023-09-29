#pragma once
#include <d3d12.h>
#include "../../D3D/D3D12Utility.hpp"
#include "../../D3D/D3D12Descriptor.hpp"
#include "../../D3D/D3D12RootSignature.hpp"
#include "../../D3D/D3D12GraphicsPipelineState.hpp"
#include "../../D3D/D3D12DepthBuffer.hpp"
#include "../../Graphics/ShaderManager.hpp"
#include "ScreenOutput.hpp"

class Model;

// TEMPORAL
// TODOs:
// Move Root Signatures and PSOs into dedicates RenderPasses

/// <summary>
/// Engine defaults to Deferred Rendering Path.<br/>
/// Class is used for initailizing context related to deferred resources and pipelines.
/// </summary>
class DeferredContext : public ScreenOutput
{
public:
	DeferredContext(std::shared_ptr<D3D::D3D12Context> pD3DContext, std::shared_ptr<gfx::ShaderManager> pShaderManager, D3D::D3D12DepthBuffer* DepthBuffer);
	~DeferredContext();

	void Initialize();
	void OnResize();
	void Release();

	static const uint32_t RenderTargetsCount{ 5 };
	
	/// <summary>
	/// Temporal.
	/// </summary>
	void PassGBuffer(Camera* pCamera, std::vector<std::unique_ptr<Model>>& Models);

	void DrawGBuffers();

	std::array<ComPtr<ID3D12Resource>, RenderTargetsCount> m_RenderTargets;
	//std::array<D3D::D3D12Descriptor, RenderTargetsCount> m_RenderDescs;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, RenderTargetsCount> m_RenderDescs;
	std::array<D3D::D3D12Descriptor, RenderTargetsCount> m_ShaderDescs;

private:
	/// <summary>
	/// 
	/// </summary>
	void CreateRenderTargets();

	/// <summary>
	/// 
	/// </summary>
	void CreateRootSignatures();

	/// <summary>
	/// Create GBuffer and Output PSO
	/// </summary>
	void CreatePipelines();

	std::array<float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	/// <summary>
	/// 
	/// </summary>
	std::shared_ptr<D3D::D3D12Context> m_D3DContext;

	/// <summary>
	/// 
	/// </summary>
	std::shared_ptr<gfx::ShaderManager> m_ShaderManager;

	D3D::D3D12DepthBuffer* m_SceneDepth;

	/// <summary>
	/// 
	/// </summary>
	std::unique_ptr<D3D::D3D12DescriptorHeap> m_DeferredHeap;

	D3D::D3D12RootSignature m_GBufferRootSignature;
	D3D::D3D12PipelineState m_GBufferPSO;

	D3D::D3D12RootSignature m_OutputRootSignature;
	D3D::D3D12PipelineState m_OutputPSO;

	std::array<DXGI_FORMAT, RenderTargetsCount> m_RenderTargetFormats{
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Depth
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Base Color
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Normal
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Metal-Roughness
		DXGI_FORMAT_R32G32B32A32_FLOAT		// World Position
	};

};
