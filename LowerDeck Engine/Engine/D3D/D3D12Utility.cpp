#include "D3D12Utility.hpp"
#include "D3D12Types.hpp"

uint32_t FRAME_INDEX{ 0 };

namespace D3D
{
	D3D12MA::ALLOCATION_DESC Utility::UploadHeap(bool bComitted)
	{
		D3D12MA::ALLOCATION_DESC desc{};
		desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		desc.Flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_BEST_FIT;
		desc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;

		if (bComitted)
			desc.Flags |= D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED;

		return desc;
	}
	D3D12_STATIC_SAMPLER_DESC Utility::CreateStaticSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_STATIC_SAMPLER_DESC desc{};
		desc.AddressU = AddressMode;
		desc.AddressV = AddressMode;
		desc.AddressW = AddressMode;
		desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		desc.ComparisonFunc = ComparsionFunc;
		desc.Filter = Filter;
		desc.MaxAnisotropy = 0;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = static_cast<float>(UINT32_MAX);
		desc.ShaderRegister = ShaderRegister;
		desc.RegisterSpace = RegisterSpace;
		desc.ShaderVisibility = Visibility;

		return desc;
	}

	std::array<D3D12_INPUT_ELEMENT_DESC, 5> Utility::GetModelInputLayout()
	{
		return std::array<D3D12_INPUT_ELEMENT_DESC, 5>{
				D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				D3D12_INPUT_ELEMENT_DESC{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				D3D12_INPUT_ELEMENT_DESC{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	std::array<D3D12_INPUT_ELEMENT_DESC, 2> Utility::GetSkyInputLayout()
	{
		return std::array<D3D12_INPUT_ELEMENT_DESC, 2>{
			D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	std::array<D3D12_INPUT_ELEMENT_DESC, 2> Utility::GetScreenOutputInputLayout()
	{
		return std::array<D3D12_INPUT_ELEMENT_DESC, 2>{
			D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}
}

namespace gfx
{
	LPCWSTR ShaderTypeToTarget(ShaderType TypeOf)
	{
		switch (TypeOf)
		{
		case ShaderType::eVertex:
			return L"vs_6_6";
		case ShaderType::ePixel:
			return L"ps_6_6";
		case ShaderType::eCompute:
			return L"cs_6_6";
		case ShaderType::eLibrary:
		case ShaderType::eRayTracing:
			return L"lib_6_6";
		case ShaderType::eMesh:
			return L"ms_6_6";
		case ShaderType::eAmplification:
			return L"as_6_6";
		case ShaderType::eGeometry:
			return L"gs_6_6";
		case ShaderType::eHull:
			return L"hs_6_6";
		case ShaderType::eDomain:
			return L"ds_6_6";
		}

		throw std::exception("Invalid shader type!");
	}
}

