#include "string.hpp"

namespace util {
	std::wstring widen(const std::string& str) {
		std::wstring result = L"";
		for (char c : str)
			result += (wchar_t)c;
		return result;
	}
}