#pragma once

template<typename T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T* instance{ nullptr };
		if (!instance)
		{
			instance = new T();
		}
		return *instance;
	}


protected:
	Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton(const Singleton&&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton& operator=(Singleton&&) = delete;
};
