#include "D3D12Utility.hpp"
#include "D3D12Types.hpp"

uint32_t FRAME_INDEX{ 0 };

namespace gfx
{
	LPCWSTR EnumToType(ShaderType TypeOf)
	{
		switch (TypeOf)
		{
		case ShaderType::eVertex:
			return L"vs_6_0";
		case ShaderType::ePixel:
			return L"ps_6_0";
		case ShaderType::eCompute:
			return L"cs_6_0";
		case ShaderType::eLibrary:
		case ShaderType::eRayTracing:
			return L"lib_6_3";
		case ShaderType::eGeometry:
			return L"gs_6_0";
		case ShaderType::eHull:
			return L"hs_6_0";
		case ShaderType::eDomain:
			return L"ds_6_0";
		}

		throw std::exception("Invalid shader type!");
	}
}
