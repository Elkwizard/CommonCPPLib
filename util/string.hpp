#pragma once

#include <string>
#include <vector>
#include <regex>
#include <type_traits>
#include <sstream>
#include <string_view>
#include <cctype>

#include <iostream>

namespace util {
	template <typename S>
	using CharType = std::decay_t<decltype(std::declval<S>()[0])>;

	std::wstring widen(const std::string& str) {
		std::wstring result = L"";
		for (char c : str)
			result += (wchar_t)c;
		return result;
	}

	template <typename T>
	struct TypeNameStruct {
		static std::string getName() {
			std::string str = __FUNCTION__;
			std::string prefix = "util::TypeNameStruct<";
			std::string suffix = ">::getName";
			return str.substr(
				prefix.size(),
				str.size() - suffix.size() - prefix.size()
			);
		}
	};

	template <typename T>
	std::string getTypeName() {
		return TypeNameStruct<T>::getName();
	} 

	template <typename T>
	std::basic_string<T> leftPad(const std::basic_string<T>& str, size_t length, T fill = ' ') {
		if (str.length() >= length) return str;
		return std::basic_string<T>(length - str.length(), fill) + str;
	}

	template <typename T>
	std::basic_string<T> rightPad(const std::basic_string<T>& str, size_t length, T fill = ' ') {
		if (str.length() > length) return str;
		return str + std::basic_string<T>(length - str.length(), fill);
	}

	template <typename S>
	std::basic_string_view<CharType<S>> trim(const S& argStr) {
		std::basic_string_view<CharType<S>> str = argStr;
		int begin = 0;
		while (begin < str.size() && std::isspace(str[begin]))
			begin++;

		if (begin == str.size()) return "";

		int end = str.size();
		while (end > 0 && std::isspace(str[end - 1]))
			end--;

		return str.substr(begin, end - begin);
	}

	template <typename T>
	std::basic_string<T> removeTrailing(const std::basic_string<T>& str, T trail = ' ') {
		for (int i = str.size() - 1; i >= 0; i--)
			if (str[i] != trail)
				return str.substr(0, i + 1);

		return { };
	}

	template <typename T>
	std::string floatToString(T v) {
		if (!v) return "0";
		return removeTrailing(removeTrailing(std::to_string(v), '0'), '.');
	}

	template <typename S>
	std::string normalizeLinebreaks(const S& str) {
		using T = CharType<S>;
		std::basic_string<T> result = "";
		for (const T& c : str)
			if (c != '\r') result += c;
		return result;
	}

	template <typename S, typename D>
	auto split(const S& argStr, D argDelim, bool pr = false) {
		using T = CharType<S>;
		std::basic_string_view<T> str = argStr;
		std::basic_string_view<T> delim = argDelim;
		std::vector<std::basic_string_view<T>> result { };
		size_t nextIndex;
		while (true) {
			nextIndex = str.find(delim);
			if (nextIndex == -1) break;
			result.push_back(str.substr(0, nextIndex));
			str = str.substr(nextIndex + delim.size());
		}
		result.push_back(str);
		return result;
	}

	template <typename S, typename D>
	auto join(const std::vector<S>& segments, D delim) {
		using T = CharType<D>;

		if (segments.empty())
			return std::basic_string<T>();
		
		std::basic_stringstream<T> result;
		result << segments[0];
		for (int i = 1; i < segments.size(); i++)
			result << delim << segments[i];
		return result.str();
	}

	template <typename T>
	std::string to_string(const T& value) {
		std::stringstream stream;
		stream << value;
		return stream.str();
	}

#if __STDC_VERSION__ >= 202311
	template <typename T, typename S>
	T parseNumber(const S& str) {
		T result;
		std::from_chars(str.data(), str.data() + str.size(), result);
		return result;
	}
#endif
}