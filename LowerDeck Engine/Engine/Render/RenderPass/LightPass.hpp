#pragma once
#include <d3dx12.h>

namespace D3D
{
	class D3D12Viewport;
	class D3D12Descriptor;
	class D3D12RootSignature;
	class D3D12PipelineState;
	class D3D12DepthBuffer;
}

class LightPass
{
public:
	/// <summary>
	/// Initializes Resources and Descriptors.
	/// </summary>
	/// <param name="pViewport"> Pointer to Scene Viewport. </param>
	/// <param name="pSceneDepth"> Pointer to Scene Depth Buffer. </param>
	/// <param name="pShaderManager"> Shared Pointer to <c>ShaderManager</c> object. </param>
	LightPass(D3D::D3D12Viewport* pViewport, D3D::D3D12DepthBuffer* pSceneDepth, std::shared_ptr<gfx::ShaderManager> pShaderManager);
	/// <summary>
	/// Releases Resources.
	/// </summary>
	~LightPass();

	/// <summary>
	/// Transits States.<br/>
	/// Clears Render Targets.<br/>
	/// Sets Render Targets.<br/>
	/// </summary>
	void BeginPass();

	/// <summary>
	/// Transits States to GENERIC_READ.
	/// </summary>
	void EndPass();

	/// <summary>
	/// Sets texture indices.
	/// </summary>
	/// <param name="GBufferIndices"> Indices for GBuffer SRV Textures. </param>
	/// <param name="IBLIndices"> Indices for Image Based Lighting SRV Textures. </param>
	void DrawData(const std::array<uint32_t, 5>& GBufferIndices, const std::array<uint32_t, 4>& IBLIndices);

	/// <summary>
	/// Releases and resizes Resources.
	/// </summary>
	void OnResize(D3D::D3D12Viewport* pViewport);
	
	/// <summary>
	/// Releases Resources.
	/// </summary>
	void Release();

	/// <summary>
	/// Getter for SRV Descriptor.
	/// </summary>
	D3D::D3D12Descriptor& OutputDescriptor() { return m_OutputDescriptor; }

private:
	/// <summary>
	/// Create Resources.<br/>
	/// Called at construction and <c>OnResize()</c>.
	/// </summary>
	void CreateTargets(D3D::D3D12Viewport* pViewport);

	Microsoft::WRL::ComPtr<ID3D12Resource>	m_OutputResource;
	D3D::D3D12Descriptor	m_OutputDescriptor;

	D3D::D3D12RootSignature m_RootSignature;
	D3D::D3D12PipelineState m_PSO;
	//CD3DX12_CPU_DESCRIPTOR_HANDLE m_OutputRTVDesc;
	D3D::D3D12Descriptor	m_OutputRTVDesc;

	D3D::D3D12DepthBuffer* m_SceneDepth{ nullptr };
};
