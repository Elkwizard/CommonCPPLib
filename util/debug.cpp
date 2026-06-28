#include "debug.hpp"

namespace util::debug {
	std::mutex coutMutex;

	void pause(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	std::string formatLineNumber(const char* file, int line) {
		return (std::string)file + ":" + leftPad(std::to_string(line), 3, '0') + " | ";
	}	
}