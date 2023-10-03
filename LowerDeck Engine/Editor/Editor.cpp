#include "../Engine/Render/Camera.hpp"
#include "../Engine/Utility/Timer.hpp"
#include "Editor.hpp"
#include "../Engine/Window/Window.hpp"
#include "../Engine/D3D/D3D12Context.hpp"
#include "../Engine/Utility/Utility.hpp"
#include "../Engine/Utility/MemoryUsage.hpp"


Editor::~Editor()
{
	//Release();
}

void Editor::Initialize(Camera* pCamera, Timer* pEngineTimer)
{
	m_Camera = pCamera;
	m_Timer = pEngineTimer;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };
	ImGui::StyleColorsDark();
	Style.WindowRounding	= 0.0f;
	Style.WindowBorderSize	= 0.0f;

	// Docking
	IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
	//IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

	

	assert(ImGui_ImplWin32_Init(Window::GetHwnd()));
	assert(ImGui_ImplDX12_Init(D3D::g_Device.Get(),
		D3D::FRAME_COUNT,
		D3D::g_RenderTargetFormat,
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

	m_MainViewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(m_MainViewport);

}

void Editor::OnFrameEnd()
{
	//ImGuiIO& IO{ ImGui::GetIO() };
	//if (IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	//{
	//	ImGuiID dockspaceID{ ImGui::GetID("MyDockSpace") };
	//	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_DockNodeHost);
	//}
	{
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File", true))
		{
			if (ImGui::MenuItem("Exit", nullptr, false))
				Window::bShouldQuit = true;
			
			ImGui::EndMenu();
		}
		
		ImGui::MenuItem("Window", nullptr, false);
		ImGui::Separator();
		// Performance data
		ImGui::Text("FPS: %d ms: %.2f", m_Timer->m_FPS, m_Timer->m_Miliseconds);
		ImGui::Separator();
		ImGui::Text("Memory: %.2f MB", utility::MemoryUsage::ReadRAM());
		ImGui::Separator();
		ImGui::Text("VRAM: %d MB", D3D::QueryAdapterMemory());

		ImGui::EndMainMenuBar();
	}

	// DEBUG
	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	const auto dims{ ImGui::GetWindowSize() };
	const auto avail{ ImGui::GetContentRegionAvail() };
	ImGui::Text("Window size: %.2f x %.2f", dims.x, dims.y);
	ImGui::Text("Available size: %.2f x %.2f", avail.x, avail.y);
	ImGui::End();

	ImGui::Begin("Log", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("TODO");
	ImGui::End();

	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

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
