#pragma once

#include <cmath>
#include <type_traits>
#include <concepts>

namespace math {
	constexpr long double PI = 3.141592653589793238462643383279502884L;
	constexpr long double E = 2.7182818284590452353602874713527L;
	constexpr long double SQRT2 = 1.4142135623730951L;
	constexpr long double SQRT3 = 1.7320508075688772L;
	
	using uint = unsigned int;

	template <typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;

	template <typename T>
	T roundTo(T value, int digits) {
		int factor = std::pow(10, digits);
		return std::round(value * factor) / factor;
	}

	template <typename T>
	T min(const T& a) {
		return a;
	}

	template <typename T>
	T max(const T& a) {
		return a;
	}

	template <typename T> requires requires (T x) { x < x; }
	T min(const T& a, const T& b) {
		return a < b ? a : b;
	}

	template <typename T> requires requires (T x) { x > x; }
	T max(const T& a, const T& b) {
		return a > b ? a : b;
	}

	template <typename T, typename... Rest>
	T min(const T& a, const T& b, const T& c, Rest ...rest) {
		return min(a, min(b, c, rest...));
	}

	template <typename T, typename... Rest>
	T max(const T& a, const T& b, const T& c, Rest ...rest) {
		return max(a, max(b, c, rest...));
	}

	template <typename T>
	T clamp(const T& n, const T& lower, const T& upper) {
		return max(lower, min(upper, n));
	}
	
	template <typename T>
	T sign(T value) {
		return (value > 0) - (value < 0);
	}

	template <typename T>
	int firstBitIndex(T value) {
		return std::log2(value & ~value);
	}

	template <typename T>
	constexpr T remap(const T& n, const T& a, const T& b, const T& a2, const T& b2) {
		return (n - a) / (b - a) * (b2 - a2) + a2;
	}
}