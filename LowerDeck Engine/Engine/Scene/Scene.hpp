#pragma once
#include "../ECS/World.hpp"
#include <vector>
#include <memory>

class Camera;
class Model;

namespace lde
{

}

class Scene
{
public:
	Scene(Camera* pCamera);
	Scene(const Scene&) = delete;
	Scene(const Scene&&) = delete;
	/// <summary>
	/// Self cleanup.
	/// </summary>
	~Scene();

	void Initialize();
	void LoadAssets();

	void Update();
	void Draw();

	void Release();
	
private:
	std::unique_ptr<ecs::World> m_World;

	Camera* m_SceneCamera{ nullptr };

	std::vector<std::unique_ptr<Model>> m_Models;
};
