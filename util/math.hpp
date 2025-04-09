#pragma once

#include <cmath>

namespace util {
	template <typename T>
	T roundTo(T value, int digits) {
		int factor = std::pow(10, digits);
		return std::round(value * factor) / factor;
	}

	template <typename T>
	T clamp(T n, T lower, T upper) {
		return std::max(lower, std::min(upper, n));
	}

	template <typename T>
	int firstBitIndex(T value) {
		return std::log2(value & ~value);
	}
}