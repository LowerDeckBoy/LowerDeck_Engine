#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "../D3D/D3D12Descriptor.hpp"
#include "../Utility/Utility.hpp"
#include "Buffer/Buffer.hpp"
#include "Buffer/ConstantBuffer.hpp"

class Camera;

namespace lde
{
	using Microsoft::WRL::ComPtr;

	class ImageBasedLighting
	{
	public:
		ImageBasedLighting(const std::string_view& Filepath);
		ImageBasedLighting(const ImageBasedLighting&) = delete;
		ImageBasedLighting(const ImageBasedLighting&&) = delete;
		~ImageBasedLighting();

		void Create(const std::string_view& Filepath);

		void Draw(Camera* pCamera);

		void Release();

		const D3D::D3D12Descriptor& SkyboxDescriptor()		 const { return m_SkyboxDescriptor; }
		const D3D::D3D12Descriptor& IrradianceDescriptor()	 const { return m_IrradianceDescriptor; }
		const D3D::D3D12Descriptor& SpecularDescriptor()	 const { return m_SpecularDescriptor; }
		const D3D::D3D12Descriptor& SpecularBRDFDescriptor() const { return m_SpBRDFDescriptor; }

	private:
		void CreateTextures(const std::string_view& Filepath);
		void CreateBuffers();

		// Convert equirectangular HDR texture into TextureCube
		void CreateCubeTexture(ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath);
		// Create 32x32 Irradiance TextureCube
		void CreateIrradiance(ID3D12RootSignature* pComputeRoot);
		// 256x256 specular reflection map
		void CreateSpecular(ID3D12RootSignature* pComputeRoot);
		void CreateSpecularBRDF(ID3D12RootSignature* pComputeRoot);


		ComPtr<ID3D12Resource> m_Skybox;
		D3D::D3D12Descriptor m_SkyboxDescriptor;

		ComPtr<ID3D12Resource> m_IrradianceMap;
		D3D::D3D12Descriptor m_IrradianceDescriptor;

		ComPtr<ID3D12Resource> m_SpecularMap;
		D3D::D3D12Descriptor m_SpecularDescriptor;
		// TEST
		D3D::D3D12Descriptor m_SpecularMippedDescriptor;

		ComPtr<ID3D12Resource> m_SpecularBRDF_LUT;
		D3D::D3D12Descriptor m_SpBRDFDescriptor;

		std::unique_ptr<gfx::VertexBuffer> m_VertexBuffer;
		std::unique_ptr<gfx::IndexBuffer> m_IndexBuffer;

		std::unique_ptr<gfx::ConstantBuffer<gfx::cbPerObject>> m_ConstBuffer;
		gfx::cbPerObject m_cbData;

		DirectX::XMMATRIX m_WorldMatrix{ DirectX::XMMatrixIdentity() };
		DirectX::XMVECTOR m_Translation{ DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
		DirectX::XMVECTOR m_Scale{ DirectX::XMVectorSet(500.0f, 500.0f, 500.0f, 0.0f) };
	};

}

