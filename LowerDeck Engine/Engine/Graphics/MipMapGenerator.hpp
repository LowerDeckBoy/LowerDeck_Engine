#pragma once
#include "Texture.hpp"
#include <DirectXMath.h>
#include "ShaderManager.hpp"

namespace D3D
{
	class D3D12Descriptor;
	class D3D12RootSignature;
	class D3D12PipelineState;
}

class MipMapGenerator
{
public:
	~MipMapGenerator()
	{
		if (m_RootSignature)
		{
			m_RootSignature->Release();
			m_RootSignature = nullptr;
		}

		if (m_ComputePipeline)
		{
			m_ComputePipeline->Release();
			m_ComputePipeline = nullptr;
		}
	}

	//https://github.com/EldarMuradov/EraEngine/blob/master/Source/Graphics/Backend/RenderBackend.cpp#L147
	void CreateComputeState(std::weak_ptr<gfx::ShaderManager> wpShaderManager);


	static ID3D12RootSignature* m_RootSignature;
	static ID3D12PipelineState* m_ComputePipeline;

	gfx::Shader* m_ComputeShader;

	// https://github.com/EldarMuradov/EraEngine/blob/master/Source/Graphics/Backend/CommandList.cpp#L233
	// https://github.com/EldarMuradov/EraEngine/blob/master/Resources/Shaders/MipMapGen_CS.hlsl
	static void Generate2D(Texture& Texture);

};
