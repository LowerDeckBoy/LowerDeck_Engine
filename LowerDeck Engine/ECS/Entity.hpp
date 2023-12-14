#pragma once
#include <EnTT/entt.hpp>
#include "World.hpp"

namespace ecs
{
	class Entity
	{
	public:
		Entity() = default;

		Entity(World* pWorld, entt::entity ID) 
			: m_World(pWorld), m_ID(ID) { }
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
			//m_World->DestroyEntity(m_ID);
			m_World = nullptr;
		}

		void Create(World* pWorld)
		{
			if (IsAlive())
				return;

			m_ID = pWorld->CreateEntity();
			m_World = pWorld;
		}

		inline entt::entity ID() const
		{
			return m_ID;
		}

		bool IsAlive()
		{
			return (m_World != nullptr);
		}

		template<typename T>
		T& GetComponent()
		{
			return m_World->Registry()->get<T>(m_ID);
		}

		template<typename T, typename... Args>
		void AddComponent(Args&&... InArgs)
		{
			m_World->Registry()->emplace<T>(this->m_ID, std::forward<Args>(InArgs)...);
		}

		template<typename T>
		void RemoveComponent()
		{
			m_World->Registry()->remove<T>(m_ID);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_World->Registry()->any_of<T>(m_ID);
		}

		bool IsValid()
		{
			return m_World->Registry()->valid(m_ID);
		}

		bool operator==(const Entity& Other) const
		{
			return (m_ID == Other.m_ID) && (m_World == Other.m_World);
		}

		bool operator!=(const Entity& Other)
		{
			return !(*this == Other);
		}

	private:
		entt::entity m_ID{};
		/// <summary>
		/// Reference to World pointer that THIS Entity belongs to. 
		/// </summary>
		World* m_World{ nullptr };
	};
}
