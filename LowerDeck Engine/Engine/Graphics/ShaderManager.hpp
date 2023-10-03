#pragma once
#include <dxcapi.h>
#include <d3d12shader.h>
#include <wrl/client.h>
#include <string_view>
#include "../D3D/D3D12Types.hpp"

namespace gfx
{
	// TODO:
	// IDxcBlob wrapper
	class Shader
	{
	};

	// TODO:
	// Add checking if .cso files are available,
	// if so, skip shader compilation

	/// <summary>
	/// Single instance for creating and compiling shader model 6.x.<br/>
	/// DXIL Libraries require to create Compiler and Library
	/// so Manager role is to create them once instead of per shader basis.<br/>
	/// Meant for <c><i>std::shared_ptr</i></c> usage.
	/// </summary>
	class ShaderManager
	{
	public:
		/// <summary>
		/// 
		/// </summary>
		ShaderManager();
		/// <summary>
		/// 
		/// </summary>
		~ShaderManager();

		/// <summary>
		/// 
		/// </summary>
		void Initialize();
		/// <summary>
		/// Release IDxc pointers.
		/// </summary>
		void Release();

		/// <summary>
		/// Compiles shader file to <c>IDxcBlob</c> using <c>IDxcCompiler3</c>.
		/// </summary>
		/// <param name="Filepath"> Path to Shader. </param>
		/// <param name="eType"> Type of shader used to fetch desired shader target. </param>
		/// <param name="EntryPoint"> Defaults to <c>main</c>. Use different <c>EntryPoint</c> if using more then one shader inside a single file. </param>
		/// <returns></returns>
		IDxcBlob* CompileDXIL(const std::string_view& Filepath, ShaderType eType, LPCWSTR EntryPoint = L"main");

	private:
		Microsoft::WRL::ComPtr<IDxcCompiler3>		m_Compiler;
		Microsoft::WRL::ComPtr<IDxcUtils>			m_DxcUtils;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>	m_IncludeHandler;
		Microsoft::WRL::ComPtr<IDxcLibrary>			m_Library;
		
	};
}
