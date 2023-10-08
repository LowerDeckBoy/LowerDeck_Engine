#include <d3d12.h>
#include "ShaderManager.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "../Utility/Utility.hpp"
#include "../Utility/FileSystem.hpp"
#include <fstream>
#include <sstream>
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

	IDxcBlob* ShaderManager::CompileDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint)
	{
		std::ifstream shaderFile(Filepath.data());
		if (!shaderFile.good())
		{
			std::string msg{ "Failed to read " + std::string(Filepath.begin(), Filepath.end()) + " shader file!\n" };
			::MessageBoxA(nullptr, msg.c_str(), "Error", MB_OK);
			throw std::runtime_error("Failed to read shader file!");
		}

		std::stringstream strStream;
		strStream << shaderFile.rdbuf();
		std::string shaderStr{ strStream.str() };

		IDxcBlobEncoding* sourceBlob{};
		ThrowIfFailed(m_Library.Get()->CreateBlobWithEncodingFromPinned(LPBYTE(shaderStr.c_str()), static_cast<uint32_t>(shaderStr.size()), 0, &sourceBlob));

		std::vector<LPCWSTR> arguments;
		// Push entry point
		arguments.push_back(L"-E");
		arguments.push_back(EntryPoint);

		// Push target
		arguments.push_back(L"-T");
		arguments.push_back(ShaderTypeToTarget(eType));

		// Push include paths
		arguments.push_back(L"-I Shaders/");
		std::wstring parentPath{ utility::ToWideString(utility::GetParentPath(Filepath)) };
		arguments.push_back(L"-I");
		arguments.push_back(parentPath.c_str());

		arguments.push_back(L"-Qstrip_debug");
		arguments.push_back(L"-Qstrip_reflect");

#if defined (_DEBUG)
		// Push debug flags
		arguments.push_back(DXC_ARG_DEBUG);
		arguments.push_back(DXC_ARG_DEBUG_NAME_FOR_SOURCE);
		arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
#else
		arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3)
#endif

		DxcBuffer buffer{ sourceBlob->GetBufferPointer(), sourceBlob->GetBufferSize(), 0 };
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
