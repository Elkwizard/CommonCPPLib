#pragma once

#include <unordered_set>

template <typename T>
std::unordered_set<T> operator +(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
	std::unordered_set<T> result;
	result.insert(a.begin(), a.end());
	result.insert(b.begin(), b.end());
	return result;
}

template <typename T>
std::unordered_set<T> operator *(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
	std::unordered_set<T> result;
	for (int el : a)
		if (b.count(el))
			result.insert(el);
	return result;
}

template <typename T>
std::unordered_set<T> operator -(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
	std::unordered_set<T> result;
	for (int el : a)
		if (!b.count(el))
			result.insert(el);
	return result;
}

template <typename T>
std::unordered_set<T> operator +(const std::unordered_set<T>& a, int other) {
	std::unordered_set<T> result = a;
	result.insert(other);
	return result;
}