#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "../D3D/D3D12Descriptor.hpp"
#include "../Utility/Utility.hpp"
#include "Buffer/Buffer.hpp"
#include "Buffer/ConstantBuffer.hpp"

class Camera;
class Texture;

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

		/// <summary>
		/// Initialize texture from file.
		/// </summary>
		/// <param name="Filepath"></param>
		void Create(const std::string_view& Filepath);

		/// <summary>
		/// Draw Skybox: plain texture.
		/// </summary>
		/// <param name="pCamera"> Pointer to Scene Camera. </param>
		void Draw(Camera* pCamera);

		/// <summary>
		/// Release Textures.
		/// </summary>
		void Release();

		const D3D::D3D12Descriptor& SkyboxDescriptor()		 const { return m_SkyboxDescriptor; }
		const D3D::D3D12Descriptor& IrradianceDescriptor()	 const { return m_IrradianceDescriptor; }
		const D3D::D3D12Descriptor& SpecularDescriptor()	 const { return m_SpecularDescriptor; }
		const D3D::D3D12Descriptor& SpecularBRDFDescriptor() const { return m_SpBRDFDescriptor; }

	private:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Filepath"></param>
		void CreateTextures(const std::string_view& Filepath);
		/// <summary>
		/// Create Vertex and Index buffers for drawing Skybox.
		/// </summary>
		void CreateBuffers();

		/// <summary>
		/// Convert equirectangular HDR texture into TextureCube
		/// </summary>
		/// <param name="pComputeRoot"> Pointer to <c>Compute Root Signature</c>. </param>
		/// <param name="Filepath"> Path to HDR texture. </param>
		void CreateCubeTexture(ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath);
		/// <summary>
		/// Create 32x32 Irradiance TextureCube.
		/// </summary>
		/// <param name="pComputeRoot"> Pointer to <c>Compute Root Signature</c>. </param>
		void CreateIrradiance(ID3D12RootSignature* pComputeRoot);
		/// <summary>
		/// 256x256 specular reflection map
		/// </summary>
		/// <param name="pComputeRoot"> Pointer to <c>Compute Root Signature</c>. </param>
		void CreateSpecular(ID3D12RootSignature* pComputeRoot);
		/// <summary>
		/// Creates Texture2D as BDRF LookUp Table.
		/// </summary>
		/// <param name="pComputeRoot"> Pointer to <c>Compute Root Signature</c>. </param>
		void CreateSpecularBRDF(ID3D12RootSignature* pComputeRoot);

		/// <summary>
		/// Post-transformed equirectangular texture.<br/>
		/// Used for drawing Skybox.
		/// </summary>
		ComPtr<ID3D12Resource> m_Skybox;
		//Texture* m_Skybox{ nullptr };
		/// <summary>
		/// SRV Descriptor for post-transform equirectangular texture usage.
		/// </summary>
		D3D::D3D12Descriptor m_SkyboxDescriptor;

		/// <summary>
		/// 32x32 Irradiance Map.
		/// </summary>
		ComPtr<ID3D12Resource> m_IrradianceMap;
		/// <summary>
		/// Descriptor for Irradiance Map usage.
		/// </summary>
		D3D::D3D12Descriptor m_IrradianceDescriptor;

		/// <summary>
		/// Specular Map for sky reflections.<br/>
		/// Note: requires a fix (mipmapping issue)!
		/// </summary>
		ComPtr<ID3D12Resource> m_SpecularMap;
		/// <summary>
		/// Descriptor for SpecularMap usage.
		/// </summary>
		D3D::D3D12Descriptor m_SpecularDescriptor;

		/// <summary>
		/// Specular BRDF LookUp Table texture.
		/// </summary>
		ComPtr<ID3D12Resource> m_SpecularBRDF_LUT;
		/// <summary>
		/// Descriptor for BRDF LUT.
		/// </summary>
		D3D::D3D12Descriptor m_SpBRDFDescriptor;

		D3D::D3D12Descriptor uavDescriptor;

		std::unique_ptr<gfx::VertexBuffer> m_VertexBuffer;
		std::unique_ptr<gfx::IndexBuffer> m_IndexBuffer;

		std::unique_ptr<gfx::ConstantBuffer<gfx::cbPerObject>> m_ConstBuffer;
		gfx::cbPerObject m_cbData;

		DirectX::XMMATRIX m_WorldMatrix{ DirectX::XMMatrixIdentity() };
		DirectX::XMVECTOR m_Translation{ DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
		DirectX::XMVECTOR m_Scale{ DirectX::XMVectorSet(500.0f, 500.0f, 500.0f, 0.0f) };
	};
}
