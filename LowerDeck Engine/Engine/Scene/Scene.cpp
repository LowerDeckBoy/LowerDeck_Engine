#include "../Render/Camera.hpp"
#include "../Render/Model/Model.hpp"
#include "Scene.hpp"


Scene::Scene(Camera* pCamera) :
	m_SceneCamera(pCamera)
{
	Initialize();
}

Scene::~Scene()
{
	Release();
}

void Scene::Initialize()
{
	LoadAssets();
}

void Scene::LoadAssets()
{
}

void Scene::Update()
{
}

void Scene::Draw()
{
}

void Scene::Release()
{
	for (auto& model : m_Models)
	{
		model.reset();
		model = nullptr;
	}
}
