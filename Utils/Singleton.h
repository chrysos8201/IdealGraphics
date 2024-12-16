#pragma once
#include <memory>

template <typename T>
class Singleton
{
private:
	static T* g_instance;

public:
	static T* Get()
	{
		if (g_instance == nullptr)
		{
			g_instance = new T();
		}

		return g_instance;
	}
};

template<typename T>
T* Singleton<T>::g_instance = nullptr;