#pragma once
#include <EnTT/entt.hpp>
#include "Component.hpp"
//#include "Entity.hpp"
#include <memory>

// TODO:
namespace ecs
{
	/// <summary>
	/// 
	/// </summary>
	class World
	{
	public:
		World() = default;
		World(const World&) = delete;
		World(const World&&) = delete;
		World operator=(const World&) = delete;
		~World();

		void Initialize();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		entt::entity CreateEntity();
		/// <summary>
		/// 
		/// </summary>
		/// <param name="Entity"></param>
		void DestroyEntity(entt::entity Entity);

		entt::registry& Registry() { return m_Registry; }

		template<typename T>
		void AddComponent(entt::entity& Entity, T& Component);

		void Release();

	private:
		entt::registry m_Registry;
		
	};

	template<typename T>
	inline void World::AddComponent(entt::entity& Entity, T& Component)
	{
		m_Registry.emplace<T>(Entity, T());
	}
}
