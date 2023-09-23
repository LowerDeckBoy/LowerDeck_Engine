#pragma once
#include "../Engine/D3D/D3D12Context.hpp"

class Camera;
class Scene;

/// <summary>
/// Render logic
/// </summary>
class Renderer
{
public:
	explicit Renderer(std::shared_ptr<Camera> pCamera);
	Renderer(const Renderer&) = delete;
	Renderer(const Renderer&&) = delete;
	~Renderer();

	void Initialize();

	void BeginFrame();
	void EndFrame();

	void RecordCommandLists();
	void Update();
	void Render();

	void OnResize();
	void Release();

protected:
	void TransitToRender();
	void TransitToPresent(D3D12_RESOURCE_STATES StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET);

	void SetHeaps(const std::vector<ID3D12DescriptorHeap*>& Heaps);
	void SetViewport();

	/// <summary>Set SwapChain Render Targets for frame presentation</summary>
	void SetRenderTarget();
	/// <summary>Clear SwapChain Render Targets and Depth Stencil</summary>
	void ClearRenderTarget();

	void DrawGUI();

private:
	std::unique_ptr<D3D::D3D12Viewport> m_SceneViewport;
	std::shared_ptr<D3D::D3D12Context> m_D3DContext;
	std::shared_ptr<Camera> m_SceneCamera;

};
