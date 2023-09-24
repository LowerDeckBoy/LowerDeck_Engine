#pragma once
#include <d3dcommon.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <string_view>

// TODO: Disable shader model 5 by default

namespace gfx
{
	/// <summary>
	/// Shader Model 5.x usage only.
	/// </summary>
	class Shader5
	{
	public:
		Shader5() {}
		Shader5(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main");
		~Shader5();

		void Create(const std::string_view& Filepath, const std::string_view& Target, const std::string_view& Entrypoint = "main");
		void Reset();

		[[nodiscard]]
		inline ID3DBlob* GetData() const noexcept { return Blob.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3DBlob> Blob;
		bool bIsInitialized{ false };

	};
}
