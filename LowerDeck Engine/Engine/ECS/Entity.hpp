#pragma once
#include <EnTT/entt.hpp>
#include "World.hpp"

namespace ecs
{
	class Entity
	{
	public:
		/// <summary> Creates new entity to given <c>World</c> object. </summary>
		/// <param name="pWorld"> Pointer to owner <c>World</c> registry. </param>
		/// <param name="Handle"> Index of this entity. </param>
		Entity(World* pWorld, entt::entity Handle) 
			: m_World(pWorld), m_Handle(Handle) { }
		/// <summary>
		/// Default copy constructor.
		/// </summary>
		/// <param name=""></param>
		Entity(const Entity&) = default;
		/// <summary>
		/// Self cleanup on destruction.
		/// </summary>
		~Entity()
		{
			m_World->DestroyEntity(m_Handle);
			m_World = nullptr;
		}

	private:
		entt::entity m_Handle{};
		/// <summary>
		/// Reference to World pointer that THIS Entity belongs to. 
		/// </summary>
		World* m_World{ nullptr };
	};
}
