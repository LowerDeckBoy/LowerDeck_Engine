#include <d3d12.h>
#include "ShaderManager.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "../Utility/Utility.hpp"
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
		ThrowIfFailed(m_Library.Get()->CreateIncludeHandler(m_IncludeHandler.ReleaseAndGetAddressOf()), "Failed to create Dxc Include Handler!");
	}

	IDxcBlob* ShaderManager::CreateDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint)
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

		IDxcBlobEncoding* textBlob{};
		ThrowIfFailed(m_Library.Get()->CreateBlobWithEncodingFromPinned(LPBYTE(shaderStr.c_str()), static_cast<uint32_t>(shaderStr.size()), 0, &textBlob));

		IDxcOperationResult* result{};

		std::wstring wstr{ std::wstring(Filepath.begin(), Filepath.end()) };
		LPCWSTR filepath{ wstr.c_str() };

		ThrowIfFailed(m_Compiler.Get()->Compile(textBlob, filepath, EntryPoint, EnumToType(eType), nullptr, 0, nullptr, 0, m_IncludeHandler.Get(), &result), "Failed to compile shader 6.x!");

		HRESULT resultCode{};
		ThrowIfFailed(result->GetStatus(&resultCode), "Failed to get shader 6.x Status!");
		if (FAILED(resultCode))
		{
			IDxcBlobEncoding* error{};
			HRESULT hResult{ result->GetErrorBuffer(&error) };
			if (FAILED(hResult))
				throw std::runtime_error("Failed to get shader error code!");

			std::vector<char> infoLog(error->GetBufferSize() + 1);
			std::memcpy(infoLog.data(), error->GetBufferPointer(), error->GetBufferSize());
			infoLog[error->GetBufferSize()] = 0;

			utility::Print("Shader Compiler error:\n");
			const auto errorMsg{ static_cast<char*>(error->GetBufferPointer()) };
			utility::Print(errorMsg);

			SAFE_DELETE(error);
			::MessageBoxA(nullptr, "Failed to compile shader!", "ERROR", MB_OK);
			throw std::runtime_error("Failed to compile shader");
		}

		IDxcBlob* blob{ nullptr };
		ThrowIfFailed(result->GetResult(&blob), "Failed to get shader 6.x IDxcBlob*!");

		SAFE_DELETE(result);
		SAFE_DELETE(textBlob);

		return blob;
	}

	constexpr LPCWSTR ShaderManager::EnumToType(ShaderType TypeOf)
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
		default:
			throw std::invalid_argument("Invalid Shader Type!");
		}
	}

	void ShaderManager::Release()
	{
		SAFE_RELEASE(m_IncludeHandler);
		SAFE_RELEASE(m_Library);
		SAFE_RELEASE(m_Compiler);
	}
}
