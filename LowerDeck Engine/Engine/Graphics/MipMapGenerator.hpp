#pragma once
#include "Texture.hpp"
#include <DirectXMath.h>
#include "ShaderManager.hpp"

namespace D3D
{
	class D3D12Descriptor;
}

struct cbMipMap
{
	uint32_t SourceMipLevel;
	uint32_t MipCount;
	uint32_t SourceDims;
	uint32_t IsSRGB;
	DirectX::XMFLOAT2 TexelSize;

};

class MipMapGenerator
{
public:
	//https://github.com/EldarMuradov/EraEngine/blob/master/Source/Graphics/Backend/RenderBackend.cpp#L147
	void CreateComputeState(std::weak_ptr<gfx::ShaderManager> wpShaderManager);

	static ID3D12RootSignature* m_RootSignature;
	static ID3D12PipelineState* m_ComputePipeline;
	static ID3D12Resource*		m_Textures[4];

	// https://github.com/EldarMuradov/EraEngine/blob/master/Source/Graphics/Backend/CommandList.cpp#L233
	// https://github.com/EldarMuradov/EraEngine/blob/master/Resources/Shaders/MipMapGen_CS.hlsl
	static void Generate2D(Texture& Texture);

	static D3D::D3D12Descriptor srvDescriptors;
	static D3D::D3D12Descriptor uavDescriptors;

};

