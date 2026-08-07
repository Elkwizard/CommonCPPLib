#pragma once

#include "util.hpp"
#include "matrix.hpp"

namespace math {
	template <typename T>
	class Range {
		public:
			const T min, max;

			Range(const T& _min, const T& _max)
			: min(_min), max(_max) { }

			Range clip(const Range& other) const {
				return {
					math::max(min, other.min),
					math::min(max, other.max)
				};
			}
	};
}