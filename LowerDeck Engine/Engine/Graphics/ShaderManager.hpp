#pragma once
#include <dxcapi.h>
#include <wrl/client.h>
#include <string_view>
#include "../D3D/D3D12Types.hpp"

namespace gfx
{
	// TODO:
	// shader wrapper
	class Shader
	{
	};

	/// <summary>
	/// Single instance for creating and compiling shader model 6.x.<br/>
	/// DXIL Libraries require to create Compiler and Library
	/// so Manager role is to create them once instead of per shader basis.<br/>
	/// Meant for <c><i>std::shared_ptr</i></c> usage.
	/// </summary>
	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();

		void Initialize();
		void Release();

		IDxcBlob* CreateDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint = L"main");

	private:
		Microsoft::WRL::ComPtr<IDxcCompiler>		m_Compiler;
		Microsoft::WRL::ComPtr<IDxcLibrary>			m_Library;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>	m_IncludeHandler;

	};
}
