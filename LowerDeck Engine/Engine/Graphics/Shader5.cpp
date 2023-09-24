#include "Shader5.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "../Utility/Utility.hpp"

#pragma comment(lib, "d3dcompiler")

namespace gfx
{
	Shader5::Shader5(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint)
	{
		Create(Filepath, Target, Entrypoint);
	}

	Shader5::~Shader5()
	{
		SAFE_RELEASE(Blob);
	}

	void Shader5::Create(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint)
	{
		if (bIsInitialized)
			return;

		uint32_t compileFlags{ 0 };
#if defined (_DEBUG) || DEBUG
		compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* error{ nullptr };
		std::wstring path{ std::wstring(Filepath.begin(), Filepath.end()) };

		const HRESULT hResult{ D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, Entrypoint.data(), Target.data(), compileFlags, 0, Blob.GetAddressOf(), &error) };

		if (error != nullptr)
		{
			utility::Print(static_cast<char*>(error->GetBufferPointer()));
		}

		if (FAILED(hResult) || Blob == nullptr)
		{
			utility::Print("Failed to compile shader!\n");
			throw std::exception();
		}

		SAFE_DELETE(error);
		bIsInitialized = true;
	}

	void Shader5::Reset()
	{
		SAFE_RELEASE(Blob);
		bIsInitialized = false;
	}

}