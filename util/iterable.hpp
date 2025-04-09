#pragma once

namespace util {
	template <typename T>
	struct Iterable {
		using Iterator = decltype(std::begin(std::declval<T>()));
		
		Iterator begin, end;

		Iterable(const T& iterable) {
			begin = std::begin(iterable);
			end = std::end(iterable);
		}
	};
}