#include "D3D12GraphicsPipelineState.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	void D3D12PipelineState::Release()
	{
		SAFE_RELEASE(m_PipelineState);
	}


	D3D12GraphicsPipelineStateBuilder::D3D12GraphicsPipelineStateBuilder(std::shared_ptr<gfx::ShaderManager> pShaderManager)
		: m_ShaderManager(pShaderManager)
	{
	}

	D3D12GraphicsPipelineStateBuilder::~D3D12GraphicsPipelineStateBuilder()
	{
		Release();
	}

	void D3D12GraphicsPipelineStateBuilder::Create(D3D12PipelineState& PSO, ID3D12RootSignature* pRootSignature, LPCWSTR DebugName)
	{
		if (PSO.Get())
			PSO.Get()->Release();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = pRootSignature;
		desc.InputLayout = { m_InputLayout.data(), static_cast<uint32_t>(m_InputLayout.size()) };
		// Set Shaders
		if (m_VertexShader)
			desc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader->GetBufferPointer(), m_VertexShader->GetBufferSize());
		if (m_PixelShader)
			desc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader->GetBufferPointer(), m_PixelShader->GetBufferSize());
		if (m_GeometryShader)
			desc.GS = CD3DX12_SHADER_BYTECODE(m_GeometryShader->GetBufferPointer(), m_GeometryShader->GetBufferSize());
		if (m_HullShader)
			desc.HS = CD3DX12_SHADER_BYTECODE(m_HullShader->GetBufferPointer(), m_HullShader->GetBufferSize());
		if (m_DomainShader)
			desc.DS = CD3DX12_SHADER_BYTECODE(m_DomainShader->GetBufferPointer(), m_DomainShader->GetBufferSize());

		// Rasterizer State
		desc.RasterizerState = m_RasterizerDesc;
		desc.RasterizerState.CullMode = m_CullMode;
		desc.RasterizerState.FillMode = m_FillMode;

		// Depth State
		desc.DepthStencilState = m_DepthDesc;

		// Blend State
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.SampleMask = UINT_MAX;
		
		// RTV
		if (!m_RenderTargetFormats.empty())
		{
			desc.NumRenderTargets = static_cast<uint32_t>(m_RenderTargetFormats.size());
			for (uint32_t i= 0; i < static_cast<uint32_t>(m_RenderTargetFormats.size()); i++)
				desc.RTVFormats[i] = m_RenderTargetFormats.at(i);
		}
		else
		{
			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		desc.NodeMask = 0;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc = { 1, 0 };

		if (m_GeometryShader)
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		else 
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		ThrowIfFailed(D3D::g_Device.Get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(PSO.GetAddressOf())), "Failed to create PipelineState!");

		PSO.Usage = PipelineType::eGraphics;
		if (DebugName)
			PSO.Get()->SetName(DebugName);
	}

	void D3D12GraphicsPipelineStateBuilder::SetRenderTargetFormats(const std::span<DXGI_FORMAT>& Formats)
	{
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.insert(m_RenderTargetFormats.begin(), Formats.begin(), Formats.end());
	}

	void D3D12GraphicsPipelineStateBuilder::SetInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
	{
		m_InputLayout.clear();
		m_InputLayout.shrink_to_fit();
		m_InputLayout.insert(m_InputLayout.begin(), InputLayout.begin(), InputLayout.end());
	}

	void D3D12GraphicsPipelineStateBuilder::SetVertexShader(const std::string_view& Filepath, LPCWSTR EntryPoint)
	{
		m_VertexShader = m_ShaderManager->CompileDXIL(Filepath, ShaderType::eVertex, EntryPoint);
	}

	void D3D12GraphicsPipelineStateBuilder::SetPixelShader(const std::string_view& Filepath, LPCWSTR EntryPoint)
	{
		m_PixelShader = m_ShaderManager->CompileDXIL(Filepath, ShaderType::ePixel, EntryPoint);
	}

	void D3D12GraphicsPipelineStateBuilder::SetGeometryShader(const std::string_view& Filepath, LPCWSTR EntryPoint)
	{
		m_GeometryShader = m_ShaderManager->CompileDXIL(Filepath, ShaderType::eGeometry, EntryPoint);
	}

	void D3D12GraphicsPipelineStateBuilder::SetHullShader(const std::string_view& Filepath, LPCWSTR EntryPoint)
	{
		m_HullShader = m_ShaderManager->CompileDXIL(Filepath, ShaderType::eHull, EntryPoint);
	}

	void D3D12GraphicsPipelineStateBuilder::SetDomainShader(const std::string_view& Filepath, LPCWSTR EntryPoint)
	{
		m_DomainShader = m_ShaderManager->CompileDXIL(Filepath, ShaderType::eDomain, EntryPoint);
	}

	void D3D12GraphicsPipelineStateBuilder::SetEnableDepth(bool bEnable)
	{
		m_DepthDesc.DepthEnable = bEnable;
		m_RasterizerDesc.DepthClipEnable = bEnable;
	}

	void D3D12GraphicsPipelineStateBuilder::SetWireframeMode(bool bEnable)
	{
		m_FillMode = bEnable ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	}

	void D3D12GraphicsPipelineStateBuilder::SetCullMode(D3D12_CULL_MODE CullMode)
	{
		m_CullMode = CullMode;
	}

	void D3D12GraphicsPipelineStateBuilder::Reset()
	{
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.shrink_to_fit();

		m_Ranges.clear();
		m_Parameters.clear();
		m_InputLayout = {};

		m_RootFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_FillMode = D3D12_FILL_MODE_SOLID;
		m_CullMode = D3D12_CULL_MODE_NONE;

		m_DepthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}

	void D3D12GraphicsPipelineStateBuilder::Release()
	{
		Reset();

		m_ShaderManager.reset();
		m_ShaderManager = nullptr;

		SAFE_DELETE(m_VertexShader);
		SAFE_DELETE(m_PixelShader);
		SAFE_DELETE(m_GeometryShader);
		SAFE_DELETE(m_HullShader);
		SAFE_DELETE(m_DomainShader);
	}
}
