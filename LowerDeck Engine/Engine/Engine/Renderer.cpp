#include "../Render/Camera.hpp"
#include "../Editor/Editor.hpp"
#include "Renderer.hpp"
#include "../Window/Window.hpp"
#include "../Utility/Utility.hpp"

bool Renderer::bVsync = true;
int32_t Renderer::SelectedRenderTarget = 0;
bool Renderer::bDrawSky = true;

Renderer::Renderer(Camera* pCamera)
{
	m_SceneCamera = pCamera;
	Initialize();
}

Renderer::~Renderer()
{

}

void Renderer::Initialize()
{
	m_SceneViewport = std::make_unique<D3D::D3D12Viewport>();

	m_D3DContext = std::make_shared<D3D::D3D12Context>();
	m_D3DContext->InitializeD3D();

	m_ShaderManager = std::make_shared<gfx::ShaderManager>();

	m_DepthStencil = std::make_unique<D3D::D3D12DepthBuffer>(m_D3DContext->GetDepthHeap(), m_D3DContext->GetSceneViewport());

	m_DeferredContext = std::make_unique<DeferredContext>(m_D3DContext, m_ShaderManager, m_DepthStencil.get());

	CreateRootSignatures();
	CreatePipelines();

	m_cbCamera = std::make_shared<gfx::ConstantBuffer<gfx::cbCameraBuffer>>(&m_cbCameraData);

	m_PointLights = std::make_unique<PointLights>();
	m_ImageBasedLighting = std::make_unique<lde::ImageBasedLighting>("Assets/Textures/HDR/newport_loft.hdr");

	m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/DamagedHelmet/DamagedHelmet.gltf", "DamagedHelmet"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/SciFiHelmet/SciFiHelmet.gltf", "SciFiHelmet"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/sponza/Sponza.gltf", "Sponza"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/cube/Cube.gltf"));
	//m_Models.emplace_back(std::make_unique<Model>("Assets/glTF/mathilda/scene.gltf"));

	D3D::ExecuteCommandLists();

}

void Renderer::RecordCommandLists()
{
	// MUST be set before actual drawing in order to gain access to bindless resources
	SetHeaps({ D3D::D3D12Context::GetMainHeap()->Heap() });

	// Forward
	//D3D::g_CommandList.Get()->SetPipelineState(m_DefaultPSO.Get());
	//D3D::g_CommandList.Get()->SetGraphicsRootSignature(m_DefaultRootSignature.Get());
	//SetViewport();
	//SetRenderTarget();
	//ClearRenderTarget();
	//m_DepthStencil->Clear();
	//for (auto& model : m_Models)
	//	model->Draw(m_SceneCamera);
	//SetViewport();
	m_DeferredContext->PassGBuffer(m_SceneCamera, m_Models);

	// NOTE: Bad solution but temporarily gets the job done
	m_DeferredContext->PassLight(m_cbCamera.get(), m_PointLights.get(), m_ImageBasedLighting.get());
	DrawSkybox();
	m_DeferredContext->PassLightEnd();

	SetViewport();
	SetRenderTarget();
	ClearRenderTarget();

	// Output viewport window | ImGuiWindowFlags_NoMoveImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
	ImGui::Begin("Scene", nullptr);
	auto viewportSize{ ImGui::GetContentRegionAvail() };
	m_ViewportWidth		= static_cast<uint32_t>(viewportSize.x);
	m_ViewportHeight	= static_cast<uint32_t>(viewportSize.y);
	ImGui::Image(reinterpret_cast<ImTextureID>(GetViewportRenderTarget(SelectedRenderTarget)), { viewportSize.x, viewportSize.y });
	ImGui::End();

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

	D3D::ExecuteCommandLists(false);

	ThrowIfFailed(D3D::g_SwapChain.Get()->Present((bVsync ? 1 : 0), 0), "Failed to present frame!");

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
	m_DeferredContext->OnResize();

	D3D::WaitForGPU();
	m_D3DContext->FlushGPU();
}

void Renderer::BeginFrame()
{
	D3D::ResetCommandLists();

	SetViewport();

	TransitToRender();

	if (m_Editor)
		m_Editor->OnFrameBegin();

}

void Renderer::EndFrame()
{
	ImGui::Begin("Camera");
	m_SceneCamera->DrawGUI();
	ImGui::End();

	DrawGUI();

	if (m_Editor)
		m_Editor->OnFrameEnd();

	TransitToPresent();
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
	ImGui::Checkbox("V-sync", &bVsync);
	ImGui::SameLine();
	ImGui::Checkbox("Draw Sky", &bDrawSky);
	ImGui::NewLine();
	ImGui::Text("Render Target:");
	ImGui::ListBox("G-Buffer", &SelectedRenderTarget, m_OutputRenderTargets.data(), static_cast<uint32_t>(m_OutputRenderTargets.size()));
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
		return m_DeferredContext->OutputDescriptor().GetGPU().ptr;
	case 1:
		return m_DeferredContext->m_ShaderDescs.at(0).GetGPU().ptr;
	case 2:
		return m_DeferredContext->m_ShaderDescs.at(1).GetGPU().ptr;
	case 3:
		return m_DeferredContext->m_ShaderDescs.at(2).GetGPU().ptr;
	case 4:
		return m_DeferredContext->m_ShaderDescs.at(3).GetGPU().ptr;
	case 5:
		return m_DeferredContext->m_ShaderDescs.at(4).GetGPU().ptr;
	}

	return m_DeferredContext->OutputDescriptor().GetGPU().ptr;
}

void Renderer::Release()
{
	m_DeferredContext.reset();

	for (auto& model : m_Models)
	{
		model.reset();
		model = nullptr;
	}

	m_cbCamera.reset();
	m_ImageBasedLighting.reset();
	m_SkyboxRS.Release();
	m_SkyboxPSO.Release();

	m_DefaultRootSignature.Release();
	m_DefaultPSO.Release();

	m_DepthStencil->Release();

	m_ShaderManager.reset();
	m_ShaderManager = nullptr;

	D3D::WaitForGPU();
	m_D3DContext->FlushGPU();

	m_D3DContext->ReleaseD3D();
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
		skyboxParams.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		skyboxParams.at(1).InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
		samplers.at(0) = D3D::Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		m_SkyboxRS.Create(skyboxParams, samplers, rootFlags, L"Image Based Lighting Root Signature");
	}

}

void Renderer::CreatePipelines()
{
	D3D::D3D12GraphicsPipelineStateBuilder* psoBuilder{ new D3D::D3D12GraphicsPipelineStateBuilder(m_ShaderManager) };
	
	//Default PSO
	{
		auto layout{ D3D::Utility::GetModelInputLayout() };
		psoBuilder->SetInputLayout(layout);
		psoBuilder->SetVertexShader("Shaders/Default_Forward.hlsl", L"VSmain");
		psoBuilder->SetPixelShader("Shaders/Default_Forward.hlsl", L"PSmain");

		psoBuilder->Create(m_DefaultPSO, m_DefaultRootSignature.Get(), L"Default PSO");
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
