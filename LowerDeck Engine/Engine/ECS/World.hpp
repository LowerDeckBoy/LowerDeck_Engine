#pragma once
#include <EnTT/entt.hpp>
#include "Component.hpp"
#include <memory>

namespace ecs
{
	/// <summary>
	/// Scene content representation.<br/>
	/// Implementes RAII.
	/// </summary>
	class World
	{
	public:
		World() = default;
		World(const World&) = delete;
		World(const World&&) = delete;
		World operator=(const World&) = delete;
		~World();

		/// <summary>
		/// Adds new entity to registry.
		/// </summary>
		/// <returns> New entity. </returns>
		entt::entity CreateEntity();
		/// <summary>
		/// Removes entity from registry.
		/// </summary>
		/// <param name="Entity"> Entity to destroy. </param>
		void DestroyEntity(entt::entity Entity);
		/// <summary>
		/// Gets underlying registry object.
		/// </summary>
		/// <returns></returns>
		entt::registry* Registry() { return m_Registry.get(); }
		
		// Check whether entity owns given component.
		template<typename T>
		bool HasComponent(entt::entity& Entity);

		// Adds given component to entity if not already owning one.
		template<typename T>
		void AddComponent(entt::entity& Entity, T& Component);

	private:
		/// <summary>
		/// Actual registry.
		/// </summary>
		std::unique_ptr<entt::registry> m_Registry;

		/// <summary>
		/// Called at constructor.<br/>
		/// Initializes registry as a pointer.
		/// </summary>
		void Initialize();
		/// <summary>
		/// Called at destructor.<br/>
		/// Cleans registry.
		/// </summary>
		void Release();

	public:
		friend class Entity;
	};

	template<typename T>
	inline bool World::HasComponent(entt::entity& Entity)
	{
		return m_Registry->any_of<T>(Entity);
	}
	
	template<typename T>
	inline void World::AddComponent(entt::entity& Entity, T& Component)
	{
		if (HasComponent<T>(Entity))
			return;
	
		m_Registry->emplace<T>(Entity, T());
	}
}
