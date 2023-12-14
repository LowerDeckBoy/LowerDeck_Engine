#include "../LowerDeck Engine/Render/Camera.hpp"
#include "../LowerDeck Engine/Render/Renderer.hpp"
#include "../LowerDeck/Timer.hpp"
#include "Editor.hpp"
#include "../LowerDeck/Window.hpp"
#include "../LowerDeck Engine/D3D/D3D12Context.hpp"
#include "../LowerDeck Engine/Utility/Utility.hpp"
#include "../LowerDeck Engine/Utility/MemoryUsage.hpp"
#include "../LowerDeck Engine/ECS/Component.hpp"
#include "ImGui/imgui_internal.h"


Editor::~Editor()
{
	//Release();
}

void Editor::Initialize(Renderer* pRenderer, Scene* pScene, Camera* pCamera, Timer* pEngineTimer)
{
	m_Renderer = pRenderer;
	m_Scene = pScene;
	m_Camera = pCamera;
	m_Timer = pEngineTimer;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& IO{ ImGui::GetIO() };
	ImGuiStyle& Style{ ImGui::GetStyle() };

	const BOOL bDarkMode{ TRUE };
	::DwmSetWindowAttribute(Window::GetHwnd(), DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));
	COLORREF captionColor{ RGB(36, 36, 36) };
	DwmSetWindowAttribute(Window::GetHwnd(), DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
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
	DrawHierarchy();
	DrawPropertyPanel();

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
	ImGui::SetNextItemWidth(400.0f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 400.0f);
	ImGui::Text("%d FPS %.2f ms", m_Timer->m_FPS, m_Timer->m_Miliseconds);
	ImGui::Text("memory: %.2fMB", utility::MemoryUsage::ReadRAM());
	ImGui::Text("VRAM: %d MB", D3D::QueryAdapterMemory());

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

void Editor::DrawHierarchy()
{
	ImGui::Begin("Hierarchy", nullptr);

	auto view = m_Scene->Registry()->view<ecs::TagComponent>();
	for (auto [entity, tag] : view.each())
	{
		ecs::Entity e(m_Scene->World(), entity);
		DrawNode(e);
		ImGui::Separator();
	}

	ImGui::End();
}

void Editor::DrawPropertyPanel()
{
	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

	if (m_SelectedEntity.IsAlive())
	{
		DrawComponentsData(m_SelectedEntity);
	}

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

void Editor::DrawNode(ecs::Entity& Entity)
{
	if (!Entity.IsValid())
	{
		LOG_WARN("Invalid Entity");
		return;
	}

	auto& tag = m_Scene->World()->Registry()->get<ecs::TagComponent>(Entity.ID());

	ImGuiTreeNodeFlags flags{ ((m_SelectedEntity == Entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding };
	bool opened{ ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)Entity.ID(), flags, tag.Name.c_str()) };
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		m_SelectedEntity = Entity;
	}

	if (opened)
	{
		ImGui::Bullet();
		ImGui::Text("Extended");
		ImGuiTreeNodeFlags flags{ ImGuiTreeNodeFlags_OpenOnArrow };
		//bool opened{ ImGui::TreeNodeEx((void*)(uint64)(uint32)Entity.ID(), flags, tag.Name.c_str())};
		//if (opened)
		//{
		//	ImGui::Bullet();
		//	ImGui::Text("Extended");
		//	ImGui::TreePop();
		//}

		ImGui::TreePop();
	}
}

void Editor::DrawComponentsData(ecs::Entity& Entity)
{
	if (Entity.HasComponent<ecs::TagComponent>())
	{
		auto& tag{ Entity.GetComponent<ecs::TagComponent>() };
		ImGui::Text(tag.Name.c_str());
		ImGui::Separator();
	}

	/*
	DrawProperties<ecs::CameraComponent>(Entity, [&](auto& Component)
		{
			DrawFloat3("Position", Component.Position);

			if (ImGui::BeginTable("Properties", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Speed");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Speed", &Component.Speed, 0.1f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Sensivity");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Sensivity", &Component.Sensivity, 0.1f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Field of View");
				ImGui::TableNextColumn();
				ImGui::DragFloat("##Field of View", &Component.FoV, 0.1f);

				ImGui::EndTable();
			}

			if (ImGui::Button("Reset"))
			{
				Component.Reset();
			}
			ImGui::Separator();
		});
		*/
	DrawProperties<ecs::TransformComponent>(Entity, [&](auto& Component)
		{
			if (ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawFloat3("Position", Component.Translation);
				DrawFloat3("Rotation", Component.Rotation);
				DrawFloat3("Scale", Component.Scale, 1.0f);
			}
		});

	DrawProperties<ecs::PositionComponent>(Entity, [&](auto& Component)
		{
			DrawFloat3("Position", Component.Position);
		});

	DrawProperties<ecs::DirectionalLightComponent>(Entity, [&](auto& Component)
		{
			DrawFloat3("Direction", Component.Direction);
			ImGui::ColorEdit4("Color", Component.Color.data());
		});

	DrawProperties<ecs::PointLightComponent>(Entity, [&](auto& Component)
		{
			DrawFloat4("Position", Component.Position);
			//DrawFloat4("Color", Component.Color);
			ImGui::ColorEdit4("Color", Component.Color.data());
		});

	DrawProperties<ecs::ColorComponent>(Entity, [&](auto& Component)
		{
			ImGui::ColorEdit4("Color", Component.Color.data());
		});
}

template<typename T, typename UI>
void Editor::DrawProperties(ecs::Entity& Entity, UI ui)
{
	if (!Entity.HasComponent<T>())
		return;

	auto& component{ Entity.GetComponent<T>() };

	ui(component);
}

void Editor::DrawFloat3(std::string Label, DirectX::XMFLOAT3& Float3, float ResetValue)
{
	if (ImGui::BeginTable("XYZ", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
	{
		ImGui::PushID(Label.c_str());
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(Label.c_str());
		ImGui::TableNextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 1.25f);
		// X
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			if (ImGui::Button("X"))
				Float3.x = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &Float3.x);
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		// Y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			if (ImGui::Button("Y"))
				Float3.y = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &Float3.y);
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		// Z
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			if (ImGui::Button("Z"))
				Float3.z = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &Float3.z);
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();
		ImGui::PopID();
		ImGui::EndTable();
	}

	ImGui::Separator();
}

void Editor::DrawFloat4(std::string Label, DirectX::XMFLOAT4& Float4, float ResetValue)
{
	if (ImGui::BeginTable("XYZW", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
	{
		ImGui::PushID(Label.c_str());
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(Label.c_str());
		ImGui::TableNextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth() * 1.25f);
		// X
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			if (ImGui::Button("X"))
				Float4.x = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &Float4.x);
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		// Y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			if (ImGui::Button("Y"))
				Float4.y = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &Float4.y);
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
		// Z
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			if (ImGui::Button("Z"))
				Float4.z = ResetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &Float4.z);
			ImGui::PopItemWidth();
		}
		// W
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			if (ImGui::Button("W"))
				Float4.w = 1.0f;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##W", &Float4.w);
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();
		ImGui::PopID();
		ImGui::EndTable();
	}

	ImGui::Separator();

}
