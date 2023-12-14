#pragma once
#include "../ECS/World.hpp"
#include "../ECS/Entity.hpp"
#include <vector>
#include <memory>

class Camera;
class Model;


class Scene
{
public:
	Scene();
	~Scene();

	void Initialize();
	//void DrawScene(ECS::World& World);

	// TODO:
	// Serialize

	ecs::World* World()
	{
		return m_World;
	}

	entt::registry* Registry() const
	{
		return m_World->Registry();
	}

	inline ecs::Entity NewEntity()
	{
		return ecs::Entity(m_World, m_World->CreateEntity());
	}

	std::shared_ptr<Camera> Camera;

	// Scene light data
	ecs::Entity DirectionalLight;
	std::vector<ecs::Entity> PointLights;


	//ECS::Entity m_CameraEntity;
	std::string SceneName{ "Default scene" };
private:
	/// @brief World to draw objects from
	ecs::World* m_World;
};
