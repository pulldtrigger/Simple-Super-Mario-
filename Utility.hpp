#pragma once

#include <cmath>
#include <random>
#include <ctime>
#include <cassert>

namespace
{
	std::default_random_engine engine(static_cast<unsigned long>(std::time(0)));
}

namespace utility
{
	static float random(float begin, float end)
	{
		assert(begin < end);
		std::uniform_real_distribution<float> dist(begin, end);
		return dist(engine);
	}

	static int random(int begin, int end)
	{
		assert(begin < end);
		std::uniform_int_distribution<int> dist(begin, end);
		return dist(engine);
	}
}