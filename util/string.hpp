#pragma once

#include <string>
#include <vector>
#include <regex>
#include <type_traits>
#include <sstream>

namespace util {
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

	template <typename T>
	std::basic_string<T> trim(const std::basic_string<T>& str) {
		return std::regex_replace(str, std::basic_regex<T>("(^(\\s*))|((\\s*)$)"), std::basic_string<T>());
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

	std::string normalizeLinebreaks(const std::string& str) {
		std::string result = "";
		for (const char& c : str)
			if (c != '\r') result += c;
		return result;
	}

	template <typename T>
	std::vector<std::basic_string<T>> split(std::basic_string<T> str, const std::basic_string<T>& delim) {
		std::vector<std::basic_string<T>> result { };
		size_t nextIndex; 
		while (true) {
			nextIndex = str.find(delim);
			if (nextIndex == -1) break;
			result.push_back(str.substr(0, nextIndex));
			str = str.substr(nextIndex + 1);
		}
		result.push_back(str);
		return result;
	}

	template <typename T>
	std::basic_string<T> join(const std::vector<std::basic_string<T>>& segments, const std::basic_string<T>& delim) {
		if (segments.empty())
			return std::basic_string<T>();
		
		std::basic_string<T> result = segments[0];
		for (int i = 1; i < segments.size(); i++)
			result += delim + segments[i];
		return result;
	}

	template <typename T>
	std::string to_string(const T& value) {
		std::stringstream stream;
		stream << value;
		return stream.str();
	}
}