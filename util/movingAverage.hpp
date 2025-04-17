#pragma once

#include <concepts>
#include <vector>
#include <numeric>

namespace util {
	template <std::floating_point T>
	class MovingAverage {
		private:
			size_t windowSize;
			std::vector<T> samples;

		public:
			MovingAverage(size_t _windowSize) {
				windowSize = _windowSize;
			}

			void addSample(T value) {
				samples.push_back(value);
				if (samples.size() > windowSize)
					samples.erase(samples.begin());
			}

			T getAverage() const {
				T sum = std::accumulate(samples.begin(), samples.end(), T(0));
				return sum / samples.size();
			}
	};
}