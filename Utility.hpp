#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <random>
#include <array>
#include <algorithm>
#include <functional>
#include <limits>
#include <cstdlib>
#include <cassert>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383f
#endif 

#ifndef M_E
#define M_E 2.71828182845904523536f
#endif 

#include "DebugText.hpp"

namespace utility
{
	template<class T = std::mt19937, std::size_t N = T::state_size>
	inline auto randomEngine() -> std::enable_if_t<!!N, T>
	{
		std::array<typename T::result_type, N> seed_data;
		thread_local std::random_device source;
		std::generate(std::begin(seed_data), std::end(seed_data), std::ref(source));
		std::seed_seq seeds(std::begin(seed_data), std::end(seed_data));
		thread_local T seeded_engine(seeds);
		return seeded_engine;
	}

	static auto& RandomEngine = randomEngine();

	template<typename C>
	inline auto randomChoice(const C& constainer)
	{
		auto& d = std::uniform_int_distribution<unsigned>(0, constainer.size() - 1);
		return constainer[random(d)];
	}

	template <class Dist>
	inline auto random(Dist& dist)
	{
		return dist(RandomEngine);
	}

	inline auto random(float begin, float end)
	{
		assert(begin < end);
		std::uniform_real_distribution<float> dist(begin, end);
		return dist(RandomEngine);
	}

	inline auto random(int begin, int end)
	{
		assert(begin < end);
		std::uniform_int_distribution<int> dist(begin, end);
		return dist(RandomEngine);
	}

	template <typename A, typename B>
	inline auto closeEnough(const A& a, const B& b, const typename std::common_type<A, B>::type& epsilon)
	{
		using std::isless;
		assert(isless(0, epsilon)); // epsilon is a part of the whole quantity
		assert(isless(epsilon, 1));
		using std::abs;
		auto const delta = abs(a - b);
		auto const x = abs(a);
		auto const y = abs(b);
		// comparable generally and |a - b| < eps * (|a| + |b|) / 2
		return isless(epsilon * y, x) && isless(epsilon * x, y) && isless((delta + delta) / (x + y), epsilon);
	}

	//calculates dot product of 2 vectors
	inline float dot(const sf::Vector2f& lv, const sf::Vector2f& rv)
	{
		return lv.x * rv.x + lv.y * rv.y;
	}
	//Returns a given vector with its length normalized to 1
	inline sf::Vector2f normalise(sf::Vector2f source)
	{
		float length = std::sqrt(dot(source, source));
		if (length != 0) source /= length;
		return source;
	}
	//returns length squared
	inline float lengthSquared(const sf::Vector2f& source)
	{
		return dot(source, source);
	}
	//Returns length of a given vector
	inline float length(const sf::Vector2f& source)
	{
		return std::sqrt(lengthSquared(source));
	}
}
