#pragma once
#include "../D3D/D3D12Context.hpp"
#include "../D3D/D3D12DepthBuffer.hpp"
#include "../D3D/D3D12RootSignature.hpp"
#include "../D3D/D3D12PipelineState.hpp"
#include "../Graphics/ShaderManager.hpp"
#include "../Graphics/Shader5.hpp"

#include "../Utility/Singleton.hpp"

#include "../Graphics/TextureManager.hpp"
#include "../Render/Model/Model.hpp"

#include "../Graphics/ImageBasedLighting.hpp"
// Temporal
#include "../Render/Lights/PointLights.hpp"

#include "../Render/ScreenOutput.hpp"
// RenderPasses
#include "../Render/RenderPass/GBufferPass.hpp"
#include "../Render/RenderPass/LightPass.hpp"

#include "../Graphics/MipMapGenerator.hpp"

#include "../ECS/Component.hpp"
#include "../ECS/Entity.hpp"


class Camera;
class Editor;
class Scene;

/// <summary>
/// Render logic
/// </summary>
class Renderer
{
public:
	Renderer(std::shared_ptr<D3D::D3D12Context> pD3DContext, Camera* pCamera);
	//Renderer(const Renderer&) = delete;
	//Renderer(const Renderer&&) = delete;
	~Renderer() {}

	void Initialize();

	void BeginFrame();
	void EndFrame();

	void RecordCommandLists();
	void Update();
	void Render();
	void Present();

	/// <summary>
	/// 
	/// </summary>
	void DrawSkybox();

	void OnResize();
	void Release();

	void SetEditor(std::shared_ptr<Editor> pEditor);

	/// <summary>
	/// Create Root Signatures to be used for rendering.
	/// </summary>
	void CreateRootSignatures();
	/// <summary>
	/// Create PSOs to be used for rendering.
	/// </summary>
	void CreatePipelines();

	/// <summary>
	/// Called when render loops and to ensure that there are NO resource "in-flight".
	/// </summary>
	void Idle();

	D3D::D3D12Context* GetD3D()
	{
		return m_D3DContext.get();
	}

protected:
	/// <summary>
	/// Change SwapChain RenderTargets to Render State.<br/>
	/// Called at <c>BeginFrame()</c>.
	/// </summary>
	void TransitToRender();
	/// <summary>
	/// Change SwapChain RenderTargets to Present State.<br/>
	/// Called at <c>EndFrame()</c>.
	/// </summary>
	/// <param name="StateBefore"></param>
	void TransitToPresent(D3D12_RESOURCE_STATES StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);

	/// <summary>
	/// Wrapper for setting desired <c>Descriptor Heaps</c> (SRV_UAV_CBV and Sampler Heaps).
	/// </summary>
	/// <param name="Heaps"> Reference to vector of Heaps. </param>
	void SetHeaps(const std::vector<ID3D12DescriptorHeap*>& Heaps);
	/// <summary>
	/// Set actual Scene Viewport - SwapChain output.
	/// </summary>
	void SetViewport();

	/// <summary>Set SwapChain Render Targets for frame presentation</summary>
	void SetRenderTarget();
	/// <summary>Clear SwapChain Render Targets and Depth Stencil</summary>
	void ClearRenderTarget();

	void DrawGUI();

public:
	/// <summary>
	/// Set Render Target to GUI scene viewport output
	/// </summary>
	/// <param name="Selected"></param>
	uint64_t GetViewportRenderTarget(int32_t Selected);

	std::array<const char*, 6> m_OutputRenderTargets{ "Shaded", "Depth", "Base Color", "Normal", "Metal-Roughness", "World Position" };

private:
	// Debug only
	// Forward States
	D3D::D3D12RootSignature m_DefaultRootSignature;
	D3D::D3D12PipelineState m_DefaultPSO;

	//
	D3D::D3D12RootSignature m_SkyboxRS;
	D3D::D3D12PipelineState m_SkyboxPSO;

private:
	std::unique_ptr<D3D::D3D12Viewport> m_SceneViewport;
	std::shared_ptr<D3D::D3D12Context> m_D3DContext;
	std::unique_ptr<D3D::D3D12DepthBuffer> m_DepthStencil;
	Camera* m_SceneCamera;
	std::shared_ptr<Editor> m_Editor;

	std::shared_ptr<TextureManager> m_TextureManager;
	std::shared_ptr<gfx::ShaderManager> m_ShaderManager;

	std::unique_ptr<ScreenOutput> m_DeferredOutput;
	//
	std::unique_ptr<GBufferPass> m_GBufferPass;
	std::unique_ptr<LightPass> m_LightPass;

	std::vector<std::unique_ptr<Model>> m_Models;
	
	std::shared_ptr<gfx::ConstantBuffer<gfx::cbCameraBuffer>> m_cbCamera;
	gfx::cbCameraBuffer m_cbCameraData{};

	std::unique_ptr<lde::ImageBasedLighting> m_ImageBasedLighting;
	// Temporal
	std::unique_ptr<PointLights> m_PointLights;
	//static 
	static MipMapGenerator m_MipGen;

public:
	static bool bVsync;

	static int32_t SelectedRenderTarget;
	static bool bDrawSky;
	
	

};
