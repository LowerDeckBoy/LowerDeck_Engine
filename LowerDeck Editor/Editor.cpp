#include "../LowerDeck Engine/Render/Camera.hpp"
#include "../LowerDeck Engine/Render/Renderer.hpp"
#include "../LowerDeck/Timer.hpp"
#include "Editor.hpp"
#include "../LowerDeck/Window.hpp"
#include "../LowerDeck Engine/D3D/D3D12Context.hpp"
#include "../LowerDeck Engine/Utility/Utility.hpp"
#include "../LowerDeck Engine/Utility/MemoryUsage.hpp"


Editor::~Editor()
{
	//Release();
}

void Editor::Initialize(Renderer* pRenderer, Camera* pCamera, Timer* pEngineTimer)
{
	m_Renderer = pRenderer;
	m_Camera = pCamera;
	m_Timer = pEngineTimer;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };
	ImGui::StyleColorsDark();
	Style.WindowRounding = 0.0f;
	Style.WindowBorderSize = 0.0f;

	// Docking
	IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	IO.ConfigFlags  |= ImGuiConfigFlags_DockingEnable;
	IO.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;

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

	LOG_INFO("LOG TESTING");

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
	DrawMenuBar();
	DrawScene();
	DrawLogs();

	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

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

void Editor::DrawMenuBar()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		ImGui::SeparatorText("File");
		if (ImGui::MenuItem("Open", "Ctrl+O"))
		{

		}
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			//auto& serializer{ Serializer::GetInstance() };
			//serializer.Serialize(m_Scene);
			LOG_INFO("Scene saved.\n");
		}

		ImGui::SeparatorText("Exit");
		if (ImGui::MenuItem("Save and Exit"))
		{
			//auto& serializer{ Serializer::GetInstance() };
			//serializer.Serialize(m_Scene);
			::PostQuitMessage(0);
		}
		if (ImGui::MenuItem("Exit"))
		{
			::PostQuitMessage(0);
		}
		ImGui::EndMenu();
	}

	//if (ImGui::BeginMenu("Edit"))
	//{
	//	ImGui::SeparatorText("Editor");
	//	if (ImGui::BeginMenu("Theme"))
	//	{
	//		if (ImGui::MenuItem("Dark Theme"))
	//		{
	//			//DarkTheme(*m_EditorStyle);
	//			LOG_INFO("Switched Editor Theme to Dark mode.");
	//		}
	//		if (ImGui::MenuItem("Light Theme"))
			{
				//LightTheme(*m_EditorStyle);
				//LOG_INFO("Switched Editor Theme to Light mode.");
			}
	//
	//		ImGui::EndMenu();
	//	}
	//
	//	ImGui::Separator();
	//	ImGui::EndMenu();
	//}

	if (ImGui::BeginMenu("View"))
	{
		// Render Outputs; GBuffer etc
		if (ImGui::BeginMenu("Render Target"))
		{
			if (ImGui::MenuItem("Shaded"))
				m_Renderer->SelectedRenderTarget = 0;
			
			if (ImGui::MenuItem("Depth"))
				m_Renderer->SelectedRenderTarget = 1; 
			
			if (ImGui::MenuItem("Base Color"))
				m_Renderer->SelectedRenderTarget = 2;
			
			if (ImGui::MenuItem("Normal"))
				m_Renderer->SelectedRenderTarget = 3;
			
			if (ImGui::MenuItem("Metal-Roughness"))//
				m_Renderer->SelectedRenderTarget = 4;

			if (ImGui::MenuItem("World Position"))
				m_Renderer->SelectedRenderTarget = 5;

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}


	auto width{ ImGui::CalcItemWidth() };
	ImGui::SetNextItemWidth(256.0f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 260.0f);
	ImGui::Text("%d FPS %.2f ms", m_Timer->m_FPS, m_Timer->m_Miliseconds);
	ImGui::Text("memory: %.2fMB", utility::MemoryUsage::ReadRAM());

	ImGui::EndMainMenuBar();
}

void Editor::DrawScene()
{
	ImGui::Begin("Scene");
	auto viewportSize{ ImGui::GetContentRegionAvail() };
	ImGui::Image((ImTextureID)m_Renderer->GetViewportRenderTarget(m_Renderer->SelectedRenderTarget), viewportSize);
	// Image
	ImGui::End();
}

void Editor::Release()
{
	m_MainFont = nullptr;
	m_MainViewport = nullptr;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Editor::DrawLogs()
{
	ImGui::Begin("Logs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Logger::ClearLogs();
	Logger::PrintAll();
	ImGui::End();
}
