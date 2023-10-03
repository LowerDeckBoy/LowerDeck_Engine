#pragma once
#include <EnTT/entt.hpp>
#include "World.hpp"

namespace ecs
{
	//class World;

	class Entity
	{
	public:
		/// <summary> </summary>
		/// <param name="pWorld"> Pointer to owner <c>World</c> registry. </param>
		/// <param name="Handle"> Index of this entity. </param>
		Entity(World* pWorld, entt::entity Handle) 
			: m_World(pWorld), m_Handle(Handle)
		{
			
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name=""></param>
		Entity(const Entity&) = default;
		/// <summary>
		/// Self cleanup on destruction.
		/// </summary>
		~Entity()
		{
			m_World->Registry().destroy(m_Handle);
			m_World = nullptr;
		}

		/// <summary>
		/// Check whether this <c>Entity</c> HAS component of type T.
		/// </summary>
		/// <typeparam name="T"> Component type. </typeparam>
		/// <returns> <b>True</b> if owns T. </returns>
		template<typename T>
		bool HasComponent()
		{
			return m_World->Registry().has<T>(m_Handle);
		}

	private:
		entt::entity m_Handle{};
		/// <summary>
		/// World reference that *THIS* Entity belongs to. 
		/// </summary>
		World* m_World{ nullptr };
	};
}
