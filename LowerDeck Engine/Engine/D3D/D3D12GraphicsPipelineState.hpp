#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include "../Graphics/ShaderManager.hpp"
#include <span>

//class Shader5;

namespace D3D
{
	/// <summary>
	/// <c>D3D12GraphicsPipelineState</c> is a builder-like class 
	/// meant for creating multiple PSOs with reusable structure
	/// and single ShaderManger for creating HLSL shaders of model 6.x.
	/// </summary>
	class D3D12GraphicsPipelineState
	{
	public:
		/// <summary>
		/// Initializes <c>ShaderManager</c> at construction.
		/// </summary>
		/// <param name="pShaderManager"></param>
		D3D12GraphicsPipelineState(std::shared_ptr<gfx::ShaderManager> pShaderManager);
		/// <summary>
		/// Resets <c>ShaderManager</c> and deletes IDxcBlobs.<br/>
		/// Calls <c>Reset()</c>.
		/// </summary>
		~D3D12GraphicsPipelineState();

		/// <summary>
		/// Create Graphics Pipeline State Object based on given data.
		/// </summary>
		/// <param name="ppPipelineState"> ID3D12PipelineState** so can be used with ComPtr </param>
		/// <param name="pRootSignature"> Root Signature to associate with PSO. </param>
		/// <param name="DebugName"> Optional debug name. </param>
		void Create(ID3D12PipelineState** ppPipelineState, ID3D12RootSignature* pRootSignature, LPCWSTR DebugName = L"");
		/// <summary>
		/// Set desired PSO ranges.
		/// </summary>
		/// <param name="Ranges"> Either std::vector or std::array. </param>
		void SetRanges(const std::span<CD3DX12_DESCRIPTOR_RANGE1>& Ranges);
		/// <summary>
		/// Set desired PSO parameters.<br/>
		/// Must match with ranges set to builder.
		/// </summary>
		/// <param name="Parameters">Either std::vector or std::array. </param>
		void SetParameters(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters);
		/// <summary>
		/// Set desired Input Layout
		/// </summary>
		/// <param name="InputLayout"> Either std::vector or std::array. </param>
		void SetInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& InputLayout);

		/// <summary></summary>
		/// <param name="Filepath"></param>
		void SetVertexShader(const std::string_view& Filepath);
		/// <summary></summary>
		/// <param name="Filepath"></param>
		void SetPixelShader(const std::string_view& Filepath);
		/// <summary></summary>
		/// <param name="Filepath"></param>
		void SetGeometryShader(const std::string_view& Filepath);
		/// <summary></summary>
		/// <param name="Filepath"></param>
		void SetHullShader(const std::string_view& Filepath);
		/// <summary></summary>
		/// <param name="Filepath"></param>
		void SetDomainShader(const std::string_view& Filepath);

		/// <summary>
		/// <b>Sets all member to default states.</b><br/>
		/// Use only when about to release <c>D3D12GraphicsPipelineState</c> object
		/// or need to create PSO much different to previous ones.<br/>
		/// Called at destructor.
		/// </summary>
		void Reset();

	private:
		void Release();

		std::shared_ptr<gfx::ShaderManager> m_ShaderManager;

		std::vector<CD3DX12_DESCRIPTOR_RANGE1>	m_Ranges;
		std::vector<CD3DX12_ROOT_PARAMETER1>	m_Parameters;
		std::vector<D3D12_INPUT_ELEMENT_DESC>	m_InputLayout;

		// Number of Render Targets is determine by size of vector.
		std::vector<DXGI_FORMAT> m_RenderTargetFormats;

		IDxcBlob* m_VertexShader	{ nullptr };
		IDxcBlob* m_PixelShader		{ nullptr };
		IDxcBlob* m_DomainShader	{ nullptr };
		IDxcBlob* m_HullShader		{ nullptr };
		IDxcBlob* m_GeometryShader	{ nullptr };

		D3D12_ROOT_SIGNATURE_FLAGS m_RootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT };

		CD3DX12_RASTERIZER_DESC m_RasterizerDesc{ CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT) };
		D3D12_CULL_MODE m_CullMode{ D3D12_CULL_MODE_BACK };
		D3D12_FILL_MODE m_FillMode{ D3D12_FILL_MODE_SOLID };

		CD3DX12_DEPTH_STENCIL_DESC m_DepthDesc	{ CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT) };
	};
} 
