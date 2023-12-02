#pragma once
#include "D3D12Types.hpp"

namespace D3D
{
	struct BufferData
	{
		void*		pData;
		uint32_t	Count;
		size_t		Size;
		uint32_t	Stride;
	};

	// TODO:
	// https://microsoft.github.io/DirectX-Specs/d3d/D3D12GPUUploadHeaps.html
	class D3D12Buffer
	{
	public:
		D3D12Buffer(BufferData Data, BufferType eType, bool bBindless = false);



	};



}
