#pragma once

#include "debug.hpp"

#include <string>
#include <fstream>
#include <filesystem>

namespace util {
	std::string directoryName(const std::string& path) {
		int location = path.find_last_of("/\\");
		if (location == -1) return ".";
		return path.substr(0, location);
	}

	std::string fileName(const std::string& path) {
		int location = path.find_last_of("/\\");
		if (location == -1) return path;
		return path.substr(location + 1);
	}

	std::string readTextFile(const std::string& path) {
		if (!std::filesystem::exists(path)) rprint("cannot find file '" + path + "'");
		std::ifstream file { path, std::ios::binary };
		std::string content {
			std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()
		};
		return content;
	}

	void writeTextFile(const std::string& path, const std::string& content) {
		std::ofstream file { path };
		file << content;
	}
}