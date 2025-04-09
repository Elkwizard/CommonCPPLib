#pragma once

#include <mutex>
#include <condition_variable>

namespace util {
	class Sync {
		public:
			std::mutex mutex;
			std::condition_variable condition;
	};
}