#pragma once
#include "Core/Core.h"
#include <random>
#include <ctime>
class RandomValue
{
public:
	RandomValue() : rng(std::random_device{}()) {}

	// min , max 포함
	uint32 nextInt(int min, int max)
	{
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng);
	}

	// min , max 포함
	double nextFloat(float min, float max)
	{
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng);
	}

	// 0 또는 1 랜덤 값 생성
	bool nextBool() 
	{
		std::bernoulli_distribution dist(0.5);
		return dist(rng);
	}

	// std::Vector에서 랜덤한 하나의 값을 선택
	template <typename T>
	T RandomElement(const std::vector<T>& contanier)
	{
		if (container.empty())
		{
			__debugbreak();
			return NULL;
		}
		std::uniform_int_distribution<size_t> dist(0, contanier.size() - 1);
		return contanier[dist(rng)];
	}

private:
	std::mt19937 rng;
};

