#include "../Render/Camera.hpp"
#include "../Render/Model/Model.hpp"
#include "Scene.hpp"
#include "../ECS/Entity.hpp"


Scene::Scene()
{
	Initialize();
}

Scene::~Scene()
{
	delete m_World;
}

void Scene::Initialize()
{
	m_World = new ecs::World();

	for (uint32_t i = 0; i < 4; i++)
	{
		ecs::Entity e(m_World, m_World->CreateEntity());
		e.AddComponent<ecs::TagComponent>("Point Light " + std::to_string(i));
		e.AddComponent<ecs::PointLightComponent>();
		PointLights.emplace_back(e);
	}
	DirectionalLight.Create(m_World);
	DirectionalLight.AddComponent<ecs::TagComponent>("Directional Light");
	DirectionalLight.AddComponent<ecs::DirectionalLightComponent>();

}
