#include "Editor.hpp"
#include "../Engine/Window/Window.hpp"
#include "../Engine/D3D/D3D12Context.hpp"
#include "../Engine/Utility/Utility.hpp"


Editor::~Editor()
{
	Release();
}

void Editor::Initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };
	Style.WindowRounding	= 0.0f;
	Style.WindowBorderSize	= 0.0f;

	// Docking
	IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
	IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

	assert(ImGui_ImplWin32_Init(Window::GetHwnd()));
	assert(ImGui_ImplDX12_Init(D3D::g_Device.Get(),
		D3D::FRAME_COUNT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D::D3D12Context::GetMainHeap()->Heap(),
		D3D::D3D12Context::GetMainHeap()->GetCPUHandle(),
		D3D::D3D12Context::GetMainHeap()->GetGPUHandle()));

	constexpr float fontSize{ 16.0f };
	m_MainFont = IO.Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode-Bold.ttf", fontSize);

	m_MainViewport = ImGui::GetMainViewport();
	m_MainViewport->Flags |= ImGuiViewportFlags_TopMost;
	m_MainViewport->Flags |= ImGuiViewportFlags_OwnedByApp;
}

void Editor::OnFrameBegin()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(m_MainFont);

	{
		ImGui::BeginMainMenuBar();
		ImGui::MenuItem("File", nullptr, false);

		ImGui::EndMainMenuBar();
	}

	m_MainViewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(m_MainViewport);
}

void Editor::OnFrameEnd()
{

	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	//m_DeviceCtx->GetCommandList()->SetDescriptorHeaps(1, m_DeviceCtx->GetMainHeap()->GetHeapAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D3D::g_CommandList.Get());
}

void Editor::Release()
{
	m_MainFont = nullptr;
	m_MainViewport = nullptr;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
