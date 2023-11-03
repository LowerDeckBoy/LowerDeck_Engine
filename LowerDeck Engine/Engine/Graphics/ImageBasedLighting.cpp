#include "ImageBasedLighting.hpp"
#include "../D3D/D3D12Device.hpp"
#include "../D3D/D3D12Utility.hpp"
#include "TextureUtility.hpp"
#include "Shader5.hpp"
#include "../Render/Camera.hpp"
#include <array>
#include <vector>

#include <directxtk12/WICTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>

#include "MipMapGenerator.hpp"

namespace lde
{
	ImageBasedLighting::ImageBasedLighting(const std::string_view& Filepath)
	{
		Create(Filepath);
	}

	ImageBasedLighting::~ImageBasedLighting()
	{
		Release();
	}

	void ImageBasedLighting::Create(const std::string_view& Filepath)
	{
		CreateBuffers();
		CreateTextures(Filepath);

		// Transition all resources to PIXEL_SHADER_RESOURCE
		const auto toPixel = [&](ID3D12GraphicsCommandList4* pCommandList) {
			std::array<D3D12_RESOURCE_BARRIER, 4> barriers{};
			barriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(m_IrradianceMap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			barriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularMap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			barriers.at(2) = CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularBRDF_LUT.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			barriers.at(3) = CD3DX12_RESOURCE_BARRIER::Transition(m_Skybox.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			pCommandList->ResourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
			};
		toPixel(D3D::g_CommandList.Get());	

	}

	void ImageBasedLighting::Draw(Camera* pCamera)
	{
		D3D::g_CommandList.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		D3D::g_CommandList.Get()->IASetVertexBuffers(0, 1, &m_VertexBuffer->View);
		D3D::g_CommandList.Get()->IASetIndexBuffer(&m_IndexBuffer->View);

		m_WorldMatrix = XMMatrixIdentity();
		m_Translation = XMVectorSet(
			DirectX::XMVectorGetX(pCamera->GetPosition()),
			DirectX::XMVectorGetY(pCamera->GetPosition()),
			DirectX::XMVectorGetZ(pCamera->GetPosition()),
			0.0f);

		m_WorldMatrix = XMMatrixScalingFromVector(m_Scale) * XMMatrixTranslationFromVector(m_Translation);
		m_ConstBuffer->Update({ XMMatrixTranspose(m_WorldMatrix * pCamera->GetViewProjection()), XMMatrixTranspose(XMMatrixIdentity()) });

		D3D::g_CommandList.Get()->SetGraphicsRootConstantBufferView(0, m_ConstBuffer->GetBuffer()->GetGPUVirtualAddress());
		D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstant(1, m_SkyboxDescriptor.Index, 0);

		D3D::g_CommandList.Get()->DrawIndexedInstanced(m_IndexBuffer->Count, 1, 0, 0, 0);

	}

	void ImageBasedLighting::Release()
	{
		SAFE_RELEASE(m_SpecularBRDF_LUT);
		SAFE_RELEASE(m_SpecularMap);
		SAFE_RELEASE(m_IrradianceMap);
		SAFE_RELEASE(m_Skybox);

		m_VertexBuffer.reset();
		m_IndexBuffer.reset();
		m_ConstBuffer.reset();
	}

	void ImageBasedLighting::CreateTextures(const std::string_view& Filepath)
	{
		ID3D12RootSignature* computeRootSignature{ nullptr };
		// Compute Root Signature
		{
			std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> ranges{};
			ranges.at(0) = { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE };
			ranges.at(1) = { D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE };

			std::array<CD3DX12_ROOT_PARAMETER1, 3> parameters{};
			// SRV
			parameters.at(0).InitAsDescriptorTable(1, &ranges.at(0));
			// UAV
			parameters.at(1).InitAsDescriptorTable(1, &ranges.at(1));
			parameters.at(2).InitAsConstants(1, 0);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
			const CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			signatureDesc.Init_1_1(static_cast<uint32_t>(parameters.size()), parameters.data(), 1, &computeSamplerDesc);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;

			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
			ThrowIfFailed(D3D::g_Device.Get()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature)));
			computeRootSignature->SetName(L"[Image Based Lighting] Compute Root Signature");

			SAFE_RELEASE(signature);
			SAFE_RELEASE(error);
		}

		CreateCubeTexture(computeRootSignature, Filepath);

		// Execute Command List BEFORE creating SRV of given Resource
		// otherwise it will cause issues when creating next SRVs
		CreateIrradiance(computeRootSignature);
		CreateSpecular(computeRootSignature);
		CreateSpecularBRDF(computeRootSignature);

		SAFE_DELETE(computeRootSignature)

	}

	void ImageBasedLighting::CreateBuffers()
	{
		std::vector<gfx::SkyVertex>* vertices = new std::vector<gfx::SkyVertex>{
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }
		};

		std::vector<uint32_t>* indices = new std::vector<uint32_t>{
			0, 1, 2, 0, 2, 3,
			4, 6, 5, 4, 7, 6,
			4, 5, 1, 4, 1, 0,
			3, 2, 6, 3, 6, 7,
			1, 5, 6, 1, 6, 2,
			4, 0, 3, 4, 3, 7
		};

		m_VertexBuffer = std::make_unique<gfx::VertexBuffer>(gfx::BufferData(vertices->data(), vertices->size(), vertices->size() * sizeof(vertices->at(0)), sizeof(vertices->at(0))));

		m_IndexBuffer = std::make_unique<gfx::IndexBuffer>(gfx::BufferData(indices->data(), indices->size(), indices->size() * sizeof(indices->at(0)), sizeof(indices->at(0))));

		m_ConstBuffer = std::make_unique<gfx::ConstantBuffer<gfx::cbPerObject>>(&m_cbData);
	}

	void ImageBasedLighting::CreateCubeTexture(ID3D12RootSignature* pComputeRoot, const std::string_view& Filepath)
	{
		// Temporal pre-transformed texture
		ComPtr<ID3D12Resource> preTransformResource;
		TextureUtility::CreateFromHDR(Filepath, preTransformResource.ReleaseAndGetAddressOf());

		//m_Skybox = new Texture(Filepath);

		D3D::D3D12Context::GetMainHeap()->Allocate(m_SkyboxDescriptor);
		TextureUtility::CreateSRV(preTransformResource.GetAddressOf(), m_SkyboxDescriptor, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		//TextureUtility::CreateSRV(preTransformResource.GetAddressOf(), m_SkyboxDescriptor, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

		const auto desc{ preTransformResource->GetDesc() };

		uint32_t cubeResolution{ 1024 };
		if (desc.Width < 1024)
			cubeResolution = 512;
		else if (desc.Width >= 2048)
			cubeResolution = 2048;
		else if (desc.Width >= 4096)
			cubeResolution = 4096;

		ComPtr<ID3D12Resource> uavTexture;
		TextureUtility::CreateResource(uavTexture.GetAddressOf(),
			TextureData(cubeResolution, cubeResolution, 6, DXGI_FORMAT_R16G16B16A16_FLOAT, 6),
			TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));

		// Temporal UAV Descriptor to hold post-transformed texture
		
		D3D::D3D12Context::GetMainHeap()->Allocate(uavDescriptor);
		TextureUtility::CreateUAV(uavTexture.GetAddressOf(), uavDescriptor, 6, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Shader to dispatch
		// Transforms coordinates into cube
		gfx::Shader5 equirectangularToCube("Shaders/Sky/EquirectangularToCube_CS.hlsl", "cs_5_1");

		// 
		ID3D12PipelineState* computePipeline;
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = pComputeRoot;
			psoDesc.CS = { equirectangularToCube.GetData()->GetBufferPointer(), equirectangularToCube.GetData()->GetBufferSize() };
			D3D::g_Device.Get()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computePipeline));
		}

		const auto barrier{ CD3DX12_RESOURCE_BARRIER::Transition(uavTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)};
		uavTexture->SetName(L"[Image Based Lighting] Environment Texture - pretransformed");

		auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			pCommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());
			pCommandList->ResourceBarrier(1, &barrier);
			pCommandList->SetComputeRootSignature(pComputeRoot);
			pCommandList->SetPipelineState(computePipeline);
			pCommandList->SetComputeRootDescriptorTable(0, m_SkyboxDescriptor.GetGPU());
			pCommandList->SetComputeRootDescriptorTable(1, uavDescriptor.GetGPU());
			pCommandList->Dispatch(cubeResolution / 32, cubeResolution / 32, 6);
			};
		dispatch(D3D::g_CommandList.Get());

		// Actual Skybox resource
		
		TextureUtility::CreateResource(m_Skybox.ReleaseAndGetAddressOf(), { cubeResolution, cubeResolution, 6, DXGI_FORMAT_R16G16B16A16_FLOAT, 6 }, TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
		m_Skybox.Get()->SetName(L"[Image Based Lighting] TextureCube Resource");

		std::array<D3D12_RESOURCE_BARRIER, 2> preCopyBarriers{};
		preCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(uavTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		preCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_Skybox.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		D3D::g_CommandList.Get()->ResourceBarrier(static_cast<uint32_t>(preCopyBarriers.size()), preCopyBarriers.data());

		D3D::g_CommandList.Get()->CopyResource(m_Skybox.Get(), uavTexture.Get());

		// States back to COMMON
		std::array<D3D12_RESOURCE_BARRIER, 2> postCopyBarriers{};
		postCopyBarriers.at(0) = CD3DX12_RESOURCE_BARRIER::Transition(uavTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
		postCopyBarriers.at(1) = CD3DX12_RESOURCE_BARRIER::Transition(m_Skybox.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		D3D::g_CommandList.Get()->ResourceBarrier(static_cast<uint32_t>(postCopyBarriers.size()), postCopyBarriers.data());

		D3D::ExecuteCommandLists(true);

		TextureUtility::CreateSRV(m_Skybox.GetAddressOf(), m_SkyboxDescriptor, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_SRV_DIMENSION_TEXTURECUBE);

		SAFE_RELEASE(preTransformResource);
		SAFE_RELEASE(uavTexture);
		SAFE_DELETE(computePipeline);

		//TEST
		//TextureUtility::
		//Texture testTexture(Filepath);
		//MipMapGenerator::Generate2D(testTexture, 2, 1);
		//D3D::ExecuteCommandLists(true);
		//D3D::WaitForGPU();
		//D3D::FlushGPU();

		
		//MipMapGenerator::Generate2D(*m_SkyboxTexture, 3, 1);

	}

	void ImageBasedLighting::CreateIrradiance(ID3D12RootSignature* pComputeRoot)
	{
		ID3D12PipelineState* pipelineState{ nullptr };
		// Pipeline
		{
			gfx::Shader5 cs("Shaders/Sky/IrradianceMap_CS.hlsl", "cs_5_1");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = pComputeRoot;
			psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
			ThrowIfFailed(D3D::g_Device.Get()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
		}

		// Resource and Descriptor creation
		{
			TextureUtility::CreateResource(m_IrradianceMap.ReleaseAndGetAddressOf(), TextureData(32, 32, 6, DXGI_FORMAT_R16G16B16A16_FLOAT),
				TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

			D3D::D3D12Context::GetMainHeap()->Allocate(m_IrradianceDescriptor);
			TextureUtility::CreateUAV(m_IrradianceMap.GetAddressOf(), m_IrradianceDescriptor, 6, DXGI_FORMAT_R16G16B16A16_FLOAT);

			m_IrradianceMap.Get()->SetName(L"[Image Based Lighting] Irradiance Map");
		}
		
		// Compute execution and resource transition
		const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			pCommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());
			pCommandList->SetComputeRootSignature(pComputeRoot);
			pCommandList->SetPipelineState(pipelineState);
			pCommandList->SetComputeRootDescriptorTable(0, m_SkyboxDescriptor.GetGPU());
			pCommandList->SetComputeRootDescriptorTable(1, m_IrradianceDescriptor.GetGPU());
			pCommandList->Dispatch(32 / 32, 32 / 32, 6);

			const auto toCommon = CD3DX12_RESOURCE_BARRIER::Transition(m_IrradianceMap.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			pCommandList->ResourceBarrier(1, &toCommon);
			};
		dispatch(D3D::g_CommandList.Get());

		D3D::ExecuteCommandLists(true);

		TextureUtility::CreateSRV(m_IrradianceMap.GetAddressOf(), m_IrradianceDescriptor, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_SRV_DIMENSION_TEXTURECUBE);

		SAFE_DELETE(pipelineState);
	}

	void ImageBasedLighting::CreateSpecular(ID3D12RootSignature* pComputeRoot)
	{
		ID3D12PipelineState* pipelineState{ nullptr };
		// Pipeline
		{
			gfx::Shader5 cs("Shaders/Sky/SpecularMap_CS.hlsl", "cs_5_1");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = pComputeRoot;
			psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
			ThrowIfFailed(D3D::g_Device.Get()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
		}

		uint16_t mips = 1;
		// Resource and Descriptor creation
		{
			TextureUtility::CreateResource(m_SpecularMap.ReleaseAndGetAddressOf(), TextureData(256, 256, 6, DXGI_FORMAT_R16G16B16A16_FLOAT, mips),
				TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

			D3D::D3D12Context::GetMainHeap()->Allocate(m_SpecularDescriptor);
			TextureUtility::CreateUAV(m_SpecularMap.GetAddressOf(), m_SpecularDescriptor, 6, DXGI_FORMAT_R16G16B16A16_FLOAT);
			m_SpecularMap.Get()->SetName(L"[Image Based Lighting] Specular Map");
		}

		// Compute execution and resource transition
		const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			pCommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());
			pCommandList->SetComputeRootSignature(pComputeRoot);
			pCommandList->SetPipelineState(pipelineState);
			pCommandList->SetComputeRootDescriptorTable(0, m_SkyboxDescriptor.GetGPU());
			pCommandList->SetComputeRootDescriptorTable(1, m_SpecularDescriptor.GetGPU());
			pCommandList->Dispatch(256 / 32, 256 / 32, 6);
		
			const auto toCommon = CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularMap.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			pCommandList->ResourceBarrier(1, &toCommon);
			};
		
		// TODO:
		/*
		const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			pCommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());
			pCommandList->SetComputeRootSignature(pComputeRoot);
			pCommandList->SetPipelineState(pipelineState);
			pCommandList->SetComputeRootDescriptorTable(0, m_SkyboxDescriptor.GetGPU());
			const float deltaRoughness = 1.0f / std::max(float(mips), 1.0f);
			for (uint32_t i = 1, size = 512; i < mips; ++i, size /= 2)
			{
				const uint32_t numGroups = std::max<uint32_t>(1, size/32);
				const float spmapRoughness = i * deltaRoughness;
				
				pCommandList->SetComputeRootDescriptorTable(1, m_SpecularDescriptor.GetGPU());
				pCommandList->SetComputeRoot32BitConstants(2, 1, &spmapRoughness, 0);
				pCommandList->Dispatch(numGroups, numGroups, 6);
			}
			
			const auto toCommon = CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularMap.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			pCommandList->ResourceBarrier(1, &toCommon);
			};
			*/
		dispatch(D3D::g_CommandList.Get());

		D3D::ExecuteCommandLists(true);
		
		TextureUtility::CreateSRV(m_SpecularMap.GetAddressOf(), m_SpecularDescriptor, mips, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_SRV_DIMENSION_TEXTURECUBE);

		SAFE_DELETE(pipelineState);
	}

	void ImageBasedLighting::CreateSpecularBRDF(ID3D12RootSignature* pComputeRoot)
	{
		ID3D12PipelineState* pipelineState{ nullptr };
		// Pipeline
		{
			gfx::Shader5 cs("Shaders/Sky/SpBRDF_LUT_CS.hlsl", "cs_5_1");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = pComputeRoot;
			psoDesc.CS = CD3DX12_SHADER_BYTECODE(cs.GetData());
			ThrowIfFailed(D3D::g_Device.Get()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
		}

		// Resource and Descriptor creation
		{
			TextureUtility::CreateResource(m_SpecularBRDF_LUT.ReleaseAndGetAddressOf(), TextureData(256, 256, 1, DXGI_FORMAT_R16G16_FLOAT),
				TextureDesc(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

			D3D::D3D12Context::GetMainHeap()->Allocate(m_SpBRDFDescriptor);
			TextureUtility::CreateUAV(m_SpecularBRDF_LUT.GetAddressOf(), m_SpBRDFDescriptor, 1, DXGI_FORMAT_R16G16_FLOAT);

			m_SpecularBRDF_LUT.Get()->SetName(L"[Image Based Lighting] Specular BRDF LUT");
		}

		// Compute execution and resource transition
		const auto dispatch = [&](ID3D12GraphicsCommandList4* pCommandList) {
			pCommandList->SetDescriptorHeaps(1, D3D::D3D12Context::GetMainHeap()->HeapAddressOf());
			pCommandList->SetComputeRootSignature(pComputeRoot);
			pCommandList->SetPipelineState(pipelineState);
			pCommandList->SetComputeRootDescriptorTable(1, m_SpBRDFDescriptor.GetGPU());
			pCommandList->Dispatch(256 / 32, 256 / 32, 6);

			const auto toCommon{ CD3DX12_RESOURCE_BARRIER::Transition(m_SpecularBRDF_LUT.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON) };
			pCommandList->ResourceBarrier(1, &toCommon);
			};
		dispatch(D3D::g_CommandList.Get());

		D3D::ExecuteCommandLists(true);

		TextureUtility::CreateSRV(m_SpecularBRDF_LUT.GetAddressOf(), m_SpBRDFDescriptor, 1, DXGI_FORMAT_R16G16_FLOAT);

		SAFE_DELETE(pipelineState);
	}
}
