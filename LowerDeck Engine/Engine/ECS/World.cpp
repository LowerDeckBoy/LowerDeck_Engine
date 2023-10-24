#include "World.hpp"

namespace ecs
{
    World::~World()
    {
        Release();
    }

    void World::Initialize()
    {
        m_Registry = std::make_unique<entt::registry>();
    }

    entt::entity World::CreateEntity()
    {
        return m_Registry->create();
    }

    void World::DestroyEntity(entt::entity Entity)
    {
        m_Registry->destroy(Entity);
    }

    void World::Release()
    {
        m_Registry->clear();
    }
}
