#include "../Render/Camera.hpp"
#include "Renderer.hpp"
#include "../Window/Window.hpp"
#include "../Utility/Utility.hpp"

Renderer::Renderer(std::shared_ptr<Camera> pCamera)
{
	m_SceneCamera = pCamera;
	Initialize();
}

Renderer::~Renderer()
{
	//Release();
}

void Renderer::Initialize()
{
	m_SceneViewport = std::make_unique<D3D::D3D12Viewport>();
	m_D3DContext = std::make_shared<D3D::D3D12Context>();
	m_D3DContext->InitializeD3D();

	D3D::ExecuteCommandLists(false);
	m_D3DContext->WaitForGPU();
}

void Renderer::RecordCommandLists()
{
	SetRenderTarget();
	ClearRenderTarget();

	SetHeaps({ D3D::D3D12Context::GetMainHeap()->Heap() });

}

void Renderer::Update()
{
}

void Renderer::Render()
{
	BeginFrame();

	RecordCommandLists();

	EndFrame();

	D3D::ExecuteCommandLists(false);

	ThrowIfFailed(D3D::g_SwapChain.Get()->Present(1, 0), "Failed to present frame!");

	m_D3DContext->MoveToNextFrame();
}

void Renderer::OnResize()
{
}

void Renderer::BeginFrame()
{
	D3D::ResetCommandLists();

	SetViewport();

	TransitToRender();
}

void Renderer::EndFrame()
{

	TransitToPresent();
}

void Renderer::TransitToRender()
{
	const auto presentToRender = CD3DX12_RESOURCE_BARRIER::Transition(D3D::GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D::g_CommandList.Get()->ResourceBarrier(1, &presentToRender);
}

void Renderer::TransitToPresent(D3D12_RESOURCE_STATES StateBefore)
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
	D3D::g_CommandList.Get()->RSSetViewports(1, &m_D3DContext->GetSceneViewport().Viewport());
	D3D::g_CommandList.Get()->RSSetScissorRects(1, &m_D3DContext->GetSceneViewport().Scissor());
}

void Renderer::SetRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3D::g_RenderTargetHeap->GetCPUHandle(), FRAME_INDEX, D3D::g_RenderTargetHeap->GetDescriptorSize());
	//const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(g_RenderTargetHeap.Get()->GetCPUDescriptorHandleForHeapStart(), D3D::m_D3DContext->FRAME_INDEX,  )
	D3D::g_CommandList.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void Renderer::ClearRenderTarget()
{
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3D::g_RenderTargetHeap->GetCPUHandle(), FRAME_INDEX, D3D::g_RenderTargetHeap->GetDescriptorSize());
	D3D::g_CommandList.Get()->ClearRenderTargetView(rtvHandle, D3D::g_ClearColor.data(), 0, nullptr);
	//const CD3DX12_CPU_DESCRIPTOR_HANDLE depthHandle(m_DepthHeap.Get()->GetCPUDescriptorHandleForHeapStart());
	//m_CommandList.Get()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Renderer::DrawGUI()
{
}

void Renderer::Release()
{
	m_D3DContext->WaitForGPU();
	m_D3DContext->FlushGPU();

	m_D3DContext->ReleaseD3D();
	::CloseHandle(D3D::g_FenceEvent);
}
