#include <d3d12.h>
#include "ShaderManager.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "../Utility/Utility.hpp"
#include "../Utility/FileSystem.hpp"
#include <vector>

#pragma comment(lib, "dxcompiler.lib")

namespace gfx
{
	ShaderManager::ShaderManager()
	{
		Initialize();
	}

	ShaderManager::~ShaderManager()
	{
		Release();
	}

	void ShaderManager::Initialize()
	{
		if (m_Compiler.Get())
			return;

		ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(m_Compiler.ReleaseAndGetAddressOf())), "Failed to create Dxc Compiler Instance!");
		ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(m_Library.ReleaseAndGetAddressOf())), "Failed to create Dxc Library Instance!");

		ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(m_DxcUtils.ReleaseAndGetAddressOf())), "Failed to create Dxc Utils Instance!");
		ThrowIfFailed(m_DxcUtils->CreateDefaultIncludeHandler(m_IncludeHandler.ReleaseAndGetAddressOf()), "Failed to create Dxc Include Handler Instance!");

	}
	//
	IDxcBlob* ShaderManager::CompileDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint)
	{
		uint32_t codePage{ DXC_CP_ACP };
		IDxcBlobEncoding* sourceBlob{};
		ThrowIfFailed(m_DxcUtils->LoadFile(utility::ToWideString(Filepath).c_str(), &codePage, &sourceBlob));

		std::wstring parentPath{ utility::ToWideString(utility::GetParentPath(Filepath)) };
		std::vector<LPCWSTR> arguments = {
			// Entry point
			L"-E", EntryPoint,
			// Target (i.e. vs_6_0)
			L"-T", ShaderTypeToTarget(eType),
			// Include paths: without them, it can cause issues when trying to do includes inside hlsl
			L"-I Shaders/", L"-I", parentPath.c_str(),
			// HLSL version: 2021 is latest
			L"-HV 2021"
		};

#if defined (_DEBUG)
		// Push debug flags
		arguments.push_back(DXC_ARG_DEBUG);
		arguments.push_back(DXC_ARG_DEBUG_NAME_FOR_SOURCE);
		arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
#else
		arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3)
#endif

		DxcBuffer buffer{ sourceBlob->GetBufferPointer(), sourceBlob->GetBufferSize(), DXC_CP_ACP };
		IDxcResult* result{ nullptr };
		ThrowIfFailed(m_Compiler.Get()->Compile(&buffer, arguments.data(), static_cast<uint32_t>(arguments.size()), m_IncludeHandler.Get(), IID_PPV_ARGS(&result)));
		
		IDxcBlobUtf8* errors{ nullptr };
		IDxcBlobUtf16* outputName{ nullptr };
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &outputName);
		if (errors && errors->GetStringLength() > 0)
		{
			utility::ErrorMessage((char*)errors->GetBufferPointer());
			throw std::runtime_error("Failed to compile shader");
		}

		IDxcBlob* blob{ nullptr };
		ThrowIfFailed(result->GetResult(&blob), "Failed to get shader 6.x IDxcBlob*!");

		SAFE_DELETE(result);
		SAFE_DELETE(sourceBlob);

		return blob;
	}

	void ShaderManager::Release()
	{
		SAFE_RELEASE(m_IncludeHandler);
		SAFE_RELEASE(m_Library);
		SAFE_RELEASE(m_DxcUtils);
		SAFE_RELEASE(m_Compiler);
	}
}
