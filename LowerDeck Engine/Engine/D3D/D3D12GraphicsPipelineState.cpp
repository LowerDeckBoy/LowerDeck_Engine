#include "D3D12GraphicsPipelineState.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

namespace D3D
{
	D3D12GraphicsPipelineState::D3D12GraphicsPipelineState(std::shared_ptr<gfx::ShaderManager> pShaderManager)
		: m_ShaderManager(pShaderManager)
	{
	}

	D3D12GraphicsPipelineState::~D3D12GraphicsPipelineState()
	{
		Release();
	}

	void D3D12GraphicsPipelineState::Create(ID3D12PipelineState** ppPipelineState, ID3D12RootSignature* pRootSignature, LPCWSTR DebugName)
	{
		if ((*(ppPipelineState)))
			(*(ppPipelineState)) = nullptr;

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

		ThrowIfFailed(D3D::g_Device.Get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(ppPipelineState)), "Failed to create PipelineState!");

		if (DebugName)
			(*ppPipelineState)->SetName(DebugName);
	}

	void D3D12GraphicsPipelineState::SetRanges(const std::span<CD3DX12_DESCRIPTOR_RANGE1>& Ranges)
	{
		m_Ranges.clear();
		m_Ranges.shrink_to_fit();
		m_Ranges.insert(m_Ranges.begin(), Ranges.begin(), Ranges.end());
	}

	void D3D12GraphicsPipelineState::SetParameters(const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters)
	{
		m_Parameters.clear();
		m_Parameters.shrink_to_fit();
		m_Parameters.insert(m_Parameters.begin(), Parameters.begin(), Parameters.end());
	}

	void D3D12GraphicsPipelineState::SetInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
	{
		m_InputLayout.clear();
		m_InputLayout.shrink_to_fit();
		m_InputLayout.insert(m_InputLayout.begin(), InputLayout.begin(), InputLayout.end());
	}

	void D3D12GraphicsPipelineState::SetVertexShader(const std::string_view& Filepath)
	{
		m_VertexShader = m_ShaderManager->CreateDXIL(Filepath, ShaderType::eVertex);
	}

	void D3D12GraphicsPipelineState::SetPixelShader(const std::string_view& Filepath)
	{
		m_PixelShader = m_ShaderManager->CreateDXIL(Filepath, ShaderType::ePixel);
	}

	void D3D12GraphicsPipelineState::SetGeometryShader(const std::string_view& Filepath)
	{
		m_GeometryShader = m_ShaderManager->CreateDXIL(Filepath, ShaderType::eGeometry);
	}

	void D3D12GraphicsPipelineState::SetHullShader(const std::string_view& Filepath)
	{
		m_HullShader = m_ShaderManager->CreateDXIL(Filepath, ShaderType::eHull);
	}

	void D3D12GraphicsPipelineState::SetDomainShader(const std::string_view& Filepath)
	{
		m_DomainShader = m_ShaderManager->CreateDXIL(Filepath, ShaderType::eDomain);
	}

	void D3D12GraphicsPipelineState::Reset()
	{
		m_Ranges.clear();
		m_Parameters.clear();
		m_InputLayout = {};

		m_RootFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_FillMode = D3D12_FILL_MODE_SOLID;
		m_CullMode = D3D12_CULL_MODE_NONE;

		m_DepthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}

	void D3D12GraphicsPipelineState::Release()
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
