#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include "../Graphics/ShaderManager.hpp"
#include <span>

//class Shader5;

namespace D3D
{
	/// <summary>
	/// Wrapper for Graphics Pipeline States.
	/// </summary>
	class D3D12PipelineState
	{
	public:
		void Release();
		
		inline ID3D12PipelineState* Get() { return m_PipelineState.Get(); }
		inline ID3D12PipelineState** GetAddressOf() { return m_PipelineState.GetAddressOf(); }

		PipelineType Usage{ PipelineType::eGraphics };
	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	};

	/// <summary>
	/// <c>D3D12GraphicsPipelineStateBuilder</c> is a builder-like class 
	/// meant for creating multiple PSOs with reusable structure
	/// and single ShaderManger for creating HLSL shaders of model 6.x.
	/// </summary>
	class D3D12GraphicsPipelineStateBuilder
	{
	public:
		/// <summary>
		/// Initializes <c>ShaderManager</c> at construction.
		/// </summary>
		/// <param name="pShaderManager"></param>
		D3D12GraphicsPipelineStateBuilder(std::shared_ptr<gfx::ShaderManager> pShaderManager);
		/// <summary>
		/// Resets <c>ShaderManager</c> and deletes IDxcBlobs.<br/>
		/// Calls <c>Reset()</c>.
		/// </summary>
		~D3D12GraphicsPipelineStateBuilder();

		/// <summary>
		/// Create Graphics Pipeline State Object based on given data.
		/// </summary>
		/// <param name="ppPipelineState"> ID3D12PipelineState** so can be used with ComPtr </param>
		/// <param name="pRootSignature"> Root Signature to associate with PSO. </param>
		/// <param name="DebugName"> Optional debug name. </param>
		void Create(D3D12PipelineState& PSO, ID3D12RootSignature* pRootSignature, LPCWSTR DebugName = L"");

		/// <summary>
		/// Set desired formats per each Render Target.<br/>
		/// If not set: only <b>one</b> Render Target of type R8G8B8A8_UNORM is set.<br/>
		/// Count of Render Targets to set is based on input Formats size.
		/// </summary>
		/// <param name="Formats"> Either <c>std::array</c> or <c>std::vector</c> of type <c>DXGI_FORMAT</c>. </param>
		void SetRenderTargetFormats(const std::span<DXGI_FORMAT>& Formats);

		/// <summary>
		/// Set desired Input Layout
		/// </summary>
		/// <param name="InputLayout"> Either std::vector or std::array. </param>
		void SetInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& InputLayout);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		/// <param name="EntryPoint"></param>
		void SetVertexShader(const std::string_view& Filepath, LPCWSTR EntryPoint = L"main");
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		/// <param name="EntryPoint"></param>
		void SetPixelShader(const std::string_view& Filepath, LPCWSTR EntryPoint = L"main");
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		/// <param name="EntryPoint"></param>
		void SetGeometryShader(const std::string_view& Filepath, LPCWSTR EntryPoint = L"main");
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		/// <param name="EntryPoint"></param>
		void SetHullShader(const std::string_view& Filepath, LPCWSTR EntryPoint = L"main");
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		/// <param name="EntryPoint"></param>
		void SetDomainShader(const std::string_view& Filepath, LPCWSTR EntryPoint = L"main");

		/// <summary>
		/// Whether to use Depth Buffer or not.
		/// </summary>
		/// <param name="bEnable"></param>
		void SetEnableDepth(bool bEnable = true);

		/// <summary>
		/// Whether to use Wireframe mode.
		/// </summary>
		/// <param name="FillMode"></param>
		void SetWireframeMode(bool bEnable = false);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="CullMode"></param>
		void SetCullMode(D3D12_CULL_MODE CullMode);

		/// <summary>
		/// <b>Sets all member to default states.</b><br/>
		/// Use only when about to release <c>D3D12GraphicsPipelineState</c> object
		/// or need to create PSO much different to previous ones.<br/>
		/// Called at destructor.
		/// </summary>
		void Reset();

	private:
		/// <summary>
		/// Called at destruction.
		/// </summary>
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
		D3D12_CULL_MODE m_CullMode{ D3D12_CULL_MODE_NONE };
		D3D12_FILL_MODE m_FillMode{ D3D12_FILL_MODE_SOLID };

		CD3DX12_DEPTH_STENCIL_DESC m_DepthDesc{ CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT) };
	};
} 
