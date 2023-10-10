#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include "../Engine//Graphics/ShaderManager.hpp"
#include <array>

namespace D3D
{
	class D3D12Viewport;
	class D3D12DepthBuffer;
	class D3D12Descriptor;
	class D3D12RootSignature;
	class D3D12PipelineState;
}

class GBufferPass
{
public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="pViewport"></param>
	/// <param name="pSceneDepth"></param>
	/// <param name="pShaderManager"></param>
	GBufferPass(D3D::D3D12Viewport* pViewport, D3D::D3D12DepthBuffer* pSceneDepth, std::shared_ptr<gfx::ShaderManager> pShaderManager);
	~GBufferPass();

	/// <summary> Total number of GBuffers. </summary>
	static const uint32_t RenderTargetsCount{ 5 };

	/// <summary>
	/// Sets Render Targets to appropriate state.<br/>
	/// Clears Render Targets.<br/>
	/// Sets PSO and RootSignature for GBuffer.<br/>
	/// </summary>
	void BeginPass();

	/// <summary>
	/// Transits RenderTargets to GENERIC_READ state.
	/// </summary>
	void EndPass();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pViewport"></param>
	void OnResize(D3D::D3D12Viewport* pViewport);

	/// <summary>
	/// Release all GBuffers.
	/// </summary>
	void Release();

	/// <summary> </summary>
	/// <returns></returns>
	std::array<D3D::D3D12Descriptor, RenderTargetsCount> GetGBuffers() { return m_SRVDescs; }

	/// <summary> </summary>
	/// <returns></returns>
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, RenderTargetsCount> GetRenderDescs() { return m_RenderTargetDescs; }

	/// <summary> </summary>
	/// <returns></returns>
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, RenderTargetsCount> GetRenderTargets() { return m_RenderTargets; }

private:
	void Initialize(D3D::D3D12Viewport* pViewport, std::shared_ptr<gfx::ShaderManager> pShaderManager);
	void CreateRenderTargets(D3D::D3D12Viewport* pViewport);

	std::array<float, 4> m_ClearColor{ 0.5f, 0.5f, 1.0f, 1.0f };
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, RenderTargetsCount> m_RenderTargets;
	std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, RenderTargetsCount> m_RenderTargetDescs{};
	std::array<D3D::D3D12Descriptor, RenderTargetsCount> m_SRVDescs;

	D3D::D3D12RootSignature m_RootSignature;
	D3D::D3D12PipelineState m_PSO;

	D3D::D3D12DepthBuffer* m_SceneDepth{ nullptr };

	/// <summary>
	/// 
	/// </summary>
	std::array<DXGI_FORMAT, RenderTargetsCount> m_RenderTargetFormats{
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Depth
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Base Color
		DXGI_FORMAT_R16G16B16A16_FLOAT,		// Normal
		DXGI_FORMAT_R8G8B8A8_UNORM,			// Metal-Roughness
		DXGI_FORMAT_R32G32B32A32_FLOAT		// World Position
	};

};
