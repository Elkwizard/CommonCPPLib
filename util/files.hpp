#pragma once

#include <string>

namespace util {
	std::string directoryName(const std::string& path);
	std::string fileName(const std::string& path);
	std::string readFile(const std::string& path);
	void writeFile(const std::string& path, const std::string& content);
}