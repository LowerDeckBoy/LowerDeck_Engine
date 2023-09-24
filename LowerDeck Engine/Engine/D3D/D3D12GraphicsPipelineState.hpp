#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include "../Graphics/ShaderManager.hpp"
#include <span>

class Shader5;
//class ShaderManager;
struct IDxcBlob;

namespace D3D
{
	/// <summary>
	/// <c>D3D12GraphicsPipelineState</c> is a builder-like class 
	/// meant for creating multiple PSOs with reusable structure
	/// and single ShaderManger for creating HLSL shaders of model 6.x.
	/// </summary>
	class D3D12GraphicsPipelineState
	{
	public:
		D3D12GraphicsPipelineState();
		D3D12GraphicsPipelineState(std::shared_ptr<gfx::ShaderManager> pShaderManager);
		~D3D12GraphicsPipelineState();

	};
}
