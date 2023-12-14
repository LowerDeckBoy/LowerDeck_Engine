#include "Camera.hpp"
#include "../LowerDeck Editor/Editor.hpp"
#include "Renderer.hpp"
#include "../LowerDeck/Window.hpp"
#include "../Utility/Utility.hpp"

bool Renderer::bVsync = true;
int32_t Renderer::SelectedRenderTarget = 0;
bool Renderer::bDrawSky = true;

MipMapGenerator Renderer::m_MipGen;

Renderer::Renderer(std::shared_ptr<D3D::D3D12Context> pD3DContext, Camera* pCamera)
{
	m_D3DContext = pD3DContext;
	m_SceneCamera = pCamera;
	Initialize();
}

//Renderer::~Renderer()
//{
//
//}

void Renderer::Initialize()
{
	m_SceneViewport = std::make_unique<D3D::D3D12Viewport>();

	// wrap into singletons
	m_ShaderManager = std::make_shared<gfx::ShaderManager>();
	m_TextureManager = std::make_shared<TextureManager>();

	m_DepthStencil = std::make_unique<D3D::D3D12DepthBuffer>(m_D3DContext->GetDepthHeap(), m_D3DContext->GetSceneViewport());

	m_DeferredOutput = std::make_unique<ScreenOutput>();

	// Render Passes
	m_GBufferPass = std::make_unique<GBufferPass>(m_D3DContext->GetSceneViewport(), m_DepthStencil.get());
	m_LightPass = std::make_unique<LightPass>(m_D3DContext->GetSceneViewport(), m_DepthStencil.get());

	CreateRootSignatures();
	CreatePipelines();

	//m_MipGen.Setup(m_ShaderManager);
	m_MipGen.CreateComputeState(m_ShaderManager);

	m_cbCamera = std::make_shared<gfx::ConstantBuffer<gfx::cbCameraBuffer>>(&m_cbCameraData);

	m_PointLights = std::make_unique<PointLights>();

	//PointLightComps.emplace_back(ecs::Entity());

	m_ImageBasedLighting = std::make_unique<lde::ImageBasedLighting>("Assets/Textures/HDR/newport_loft.hdr");
	//m_ImageBasedLighting = std::make_unique<lde::ImageBasedLighting>("Assets/Textures/HDR/environment.hdr");
	//m_ImageBasedLighting = std::make_unique<lde::ImageBasedLighting>("Assets/Textures/HDR/kloppenheim_06_puresky_4k.hdr");
	//m_ImageBasedLighting = std::make_unique<lde::ImageBasedLighting>("Assets/Textures/HDR/satara_night_4k.hdr");

	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/DamagedHelmet/DamagedHelmet.gltf", "DamagedHelmet"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/SciFiHelmet/SciFiHelmet.gltf", "SciFiHelmet"));
	m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/sponza/Sponza.gltf", "Sponza"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/MetalRoughSpheres/MetalRoughSpheres.gltf", "ballz"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/cube/Cube.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/mathilda/scene.gltf"));
	
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/Bistro_v5_2/Bistro_v5_2/BistroExterior.fbx", "Bistro"));

	D3D::ExecuteCommandLists();

}

void Renderer::RecordCommandLists()
{
	// MUST be set before actual drawing in order to gain access to bindless resources
	SetHeaps({ D3D::D3D12Context::GetMainHeap()->Heap() });

	// GBuffer pass
	{
		m_GBufferPass->BeginPass();

		for (const auto& model : m_Models)
			model->Draw(m_SceneCamera);

		m_GBufferPass->EndPass();
	}
	
	// Light Pass
	{
		m_LightPass->BeginPass();
	
		// TODO: Gotta clean it up
		std::array<uint32_t, 5> gbufferIndices { m_GBufferPass->GetGBuffers().at(0).Index, m_GBufferPass->GetGBuffers().at(1).Index, m_GBufferPass->GetGBuffers().at(2).Index, m_GBufferPass->GetGBuffers().at(3).Index, m_GBufferPass->GetGBuffers().at(4).Index };
		std::array<uint32_t, 4> iblIndices{ m_ImageBasedLighting->SkyboxDescriptor().Index, m_ImageBasedLighting->IrradianceDescriptor().Index, m_ImageBasedLighting->SpecularDescriptor(). Index, m_ImageBasedLighting->SpecularBRDFDescriptor().Index };
	
		m_LightPass->DrawData(gbufferIndices, iblIndices);
	
		D3D::g_CommandList.Get()->SetGraphicsRootConstantBufferView(0, m_cbCamera->GetBuffer()->GetGPUVirtualAddress());
		D3D::g_CommandList.Get()->SetGraphicsRootConstantBufferView(3, m_PointLights->m_cbPointLights->GetBuffer()->GetGPUVirtualAddress());
		//D3D::g_CommandList.Get()->SetGraphicsRoot32BitConstants()
		m_DeferredOutput->Draw();
	
		D3D::TransitResource(m_DepthStencil->Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	
		DrawSkybox();
	
		m_LightPass->EndPass();
	}
	
	SetViewport();
	SetRenderTarget();
	ClearRenderTarget();

	// Output viewport window 
	//ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	//auto viewportSize{ ImGui::GetContentRegionAvail() };
	//ImGui::Image(reinterpret_cast<ImTextureID>(GetViewportRenderTarget(SelectedRenderTarget)), { viewportSize.x, viewportSize.y });
	//ImGui::End();

}

void Renderer::Update()
{
	m_cbCameraData = {
		m_SceneCamera->GetPositionFloat(),
		m_SceneCamera->GetViewFloats(),
		m_SceneCamera->GetProjectionFloats(),
		XMMatrixInverse(nullptr, m_SceneCamera->GetView()),
		XMMatrixInverse(nullptr, m_SceneCamera->GetProjection()),
		m_SceneCamera->GetZNear(),
		m_SceneCamera->GetZFar()
	};
	m_cbCamera->Update(m_cbCameraData);

	m_PointLights->UpdateLights();
}

void Renderer::Render()
{
	BeginFrame();

	RecordCommandLists();

	EndFrame();
}

void Renderer::Present()
{
	TransitToPresent();
	D3D::ExecuteCommandLists(false);

	ThrowIfFailed(D3D::g_SwapChain.Get()->Present((bVsync ? 1 : 0), 0));

	m_D3DContext->MoveToNextFrame();
}

void Renderer::DrawSkybox()
{
	if (!bDrawSky)
		return;

	D3D::SetRootSignature(m_SkyboxRS);
	D3D::SetPSO(m_SkyboxPSO);
	m_ImageBasedLighting->Draw(m_SceneCamera);

}

void Renderer::OnResize()
{
	m_D3DContext->OnResize();
	m_DepthStencil->OnResize(m_D3DContext->GetDepthHeap(), m_D3DContext->GetSceneViewport());

	m_GBufferPass->OnResize(m_D3DContext->GetSceneViewport());
	m_LightPass->OnResize(m_D3DContext->GetSceneViewport());

	D3D::WaitForGPU();
	m_D3DContext->FlushGPU();
}

void Renderer::BeginFrame()
{
	D3D::ResetCommandLists();

	SetViewport();

	TransitToRender();

	//if (m_Editor)
	//	m_Editor->OnFrameBegin();

}

void Renderer::EndFrame()
{
	//DrawGUI();
	//
	//if (m_Editor)
	//	m_Editor->OnFrameEnd();

	//TransitToPresent();
}

void Renderer::TransitToRender()
{
	const auto presentToRender = CD3DX12_RESOURCE_BARRIER::Transition(D3D::GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D::g_CommandList.Get()->ResourceBarrier(1, &presentToRender);
}

void Renderer::TransitToPresent(D3D12_RESOURCE_STATES)
{
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(D3D::GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	D3D::g_CommandList.Get()->ResourceBarrier(1, &barrier);
}

void Renderer::SetHeaps(const std::vector<ID3D12DescriptorHeap*>& Heaps)
{
	D3D::g_CommandList.Get()->SetDescriptorHeaps(static_cast<uint32_t>(Heaps.size()), Heaps.data());
}

void Renderer::SetViewport()
{
	D3D::g_CommandList.Get()->RSSetViewports(1, &m_D3DContext->GetSceneViewport()->Viewport());
	D3D::g_CommandList.Get()->RSSetScissorRects(1, &m_D3DContext->GetSceneViewport()->Scissor());
}

void Renderer::SetRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3D::g_RenderTargetHeap->GetCPUHandle(), FRAME_INDEX, D3D::g_RenderTargetHeap->GetDescriptorSize());
	const auto depthHandle{ m_DepthStencil->DSV().GetCPU() };
	D3D::g_CommandList.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);
}

void Renderer::ClearRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3D::g_RenderTargetHeap->GetCPUHandle(), FRAME_INDEX, D3D::g_RenderTargetHeap->GetDescriptorSize());
	D3D::g_CommandList.Get()->ClearRenderTargetView(rtvHandle, D3D::g_ClearColor.data(), 0, nullptr);
}

void Renderer::DrawGUI()
{
	ImGui::Begin("Scene properties");
	if (ImGui::CollapsingHeader("Properties"))
	{
		ImGui::Checkbox("V-sync", &bVsync);
		ImGui::SameLine();
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		m_SceneCamera->DrawGUI();

	}

	ImGui::End();

	m_PointLights->DrawGUI();

	for (auto& model : m_Models)
		model->DrawGUI();
}

uint64_t Renderer::GetViewportRenderTarget(int32_t Selected)
{
	switch (Selected)
	{
	case 0:
		return m_LightPass->OutputDescriptor().GetGPU().ptr;
	case 1:
		return m_GBufferPass->GetGBuffers().at(0).GetGPU().ptr;
	case 2:
		return m_GBufferPass->GetGBuffers().at(1).GetGPU().ptr;
	case 3:
		return m_GBufferPass->GetGBuffers().at(2).GetGPU().ptr;
	case 4:
		return m_GBufferPass->GetGBuffers().at(3).GetGPU().ptr;
	case 5:
		return m_GBufferPass->GetGBuffers().at(4).GetGPU().ptr;
	}

	return m_LightPass->OutputDescriptor().GetGPU().ptr;
}

void Renderer::Release()
{
	for (auto& model : m_Models)
	{
		model.reset();
		model = nullptr;
	}

	m_ShaderManager.reset();
	m_ShaderManager = nullptr;
}

void Renderer::SetEditor(std::shared_ptr<Editor> pEditor)
{
	m_Editor = pEditor;
}

void Renderer::CreateRootSignatures()
{
	D3D12_ROOT_SIGNATURE_FLAGS rootFlags{ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED };
	//
	{
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;

		std::vector<CD3DX12_ROOT_PARAMETER1> parameters(2);
		// Per Object Matrices
		parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// Camera Buffer
		parameters.at(1).InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
		samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		m_DefaultRootSignature.Create(parameters, samplers, rootFlags, L"Default Root Signature");
	}

	// Image Based Lighting
	{
		std::vector<CD3DX12_ROOT_PARAMETER1> skyboxParams(2);
		// Per Object Matrices
		skyboxParams.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// Texture Indices
		skyboxParams.at(1).InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
		samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		m_SkyboxRS.Create(skyboxParams, samplers, rootFlags, L"Image Based Lighting Root Signature");
	}

}

void Renderer::CreatePipelines()
{
	auto* psoBuilder{ new D3D::D3D12GraphicsPipelineStateBuilder() };
	
	//Default PSO
	{
		auto layout{ D3D::Utility::GetModelInputLayout() };
		psoBuilder->SetInputLayout(layout);
		psoBuilder->SetVertexShader("Shaders/Forward.hlsl", L"VSmain");
		psoBuilder->SetPixelShader("Shaders/Forward.hlsl", L"PSmain");
		//psoBuilder->SetWireframeMode(true);

		//psoBuilder->Create(m_DefaultPSO, m_DefaultRootSignature.Get(), L"Default PSO");
		psoBuilder->Reset();
	}

	// Image Based Lighting
	{
		auto layout{ D3D::Utility::GetSkyInputLayout() };
		psoBuilder->SetInputLayout(layout);
		psoBuilder->SetVertexShader("Shaders/Sky/Skybox.hlsl", L"VSmain");
		psoBuilder->SetPixelShader("Shaders/Sky/Skybox.hlsl", L"PSmain");
		psoBuilder->SetEnableDepth(true);
		//psoBuilder->setcu
		psoBuilder->Create(m_SkyboxPSO, m_SkyboxRS.Get(), L"Image Based Lighting PSO");
		psoBuilder->Reset();
	}

	psoBuilder->Reset();
	delete psoBuilder;
}

void Renderer::Idle()
{
	D3D::WaitForGPU();
	m_D3DContext->FlushGPU();
}
