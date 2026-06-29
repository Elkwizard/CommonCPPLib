#pragma once

#include "string.hpp"
#include "windows.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <mutex>
#include <sstream>
#include <chrono>
#include <thread>

namespace util {
	namespace debug {
		extern std::mutex coutMutex;

		void pause(int ms);

		std::string formatLineNumber(const char* file, int line);
		
		template <int N>
		inline void logValue(const char* expr, const char (&value)[N], bool lock) {
			if (lock) coutMutex.lock();
			std::cout << value << std::endl;
			std::cout.flush();
			if (lock) coutMutex.unlock();
		}
		
		template <int N>
		inline void logValue(const char* expr, const wchar_t (&value)[N], bool lock) {
			if (lock) coutMutex.lock();
			std::wcout << value << std::endl;
			std::wcout.flush();
			if (lock) coutMutex.unlock();
		}

		template <typename T>
		inline void logValue(const char* expr, const T& value, bool lock) {
			if (lock) coutMutex.lock();
			std::cout << expr << ": " << value << std::endl;
			std::cout.flush();
			if (lock) coutMutex.unlock();
		}

		template <>
		inline void logValue(const char* expr, const std::wstring& value, bool lock) {
			if (lock) coutMutex.lock();
			std::cout << expr << ": ";
			std::cout.flush();
			std::wcout << value;
			std::wcout.flush();
			std::cout << std::endl;
			std::cout.flush();
			if (lock) coutMutex.unlock();
		}
		
		template <typename T>
		inline void uniqueLogValue(const char* expr, const T& value) {
			int maxValue = 100;
			std::string str = std::to_string(abs(rand() % maxValue));
			str = std::string(std::to_string(maxValue - 1).size() - str.length(), '0') + str;
			std::unique_lock guard { coutMutex };
			std::cout << str << " | ";
			std::cout.flush();
			logValue(expr, value, false);
		}
	}
}

// #define LOG_LINE_NUMBERS

#ifdef LOG_LINE_NUMBERS

#define FORMATTED_LN util::debug::formatLineNumber(__FILE__, __LINE__)
#define log(v) util::debug::logValue((FORMATTED_LN + #v).c_str(), (v), true)
#define ulog(v) util::debug::uniqueLogValue((FORMATTED_LN + #v).c_str(), (v));
#define rlog(v) std::cout << FORMATTED_LN << v << std::endl

#else

#define log(v) util::debug::logValue(#v, (v), true)
#define ulog(v) util::debug::uniqueLogValue(#v, (v));
#define rlog(v) std::cout << v << std::endl

#endif

#define alert(v) MessageBoxA(NULL, std::to_string(v).c_str(), "Alert", MB_OK)
#define palert(v) do { log(v); alert(v); } while (false)

#ifdef LOG_CONSTRUCTION

#define CONSTRUCT(type) std::cout << "Constructed " #type << std::endl
#define DESTRUCT(type) std::cout << "Destructed " #type << std::endl

#else

#define CONSTRUCT(type) #type
#define DESTRUCT(type) #type

#endif