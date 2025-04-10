#pragma once

#include "debug.hpp"
#include "string.hpp"
#include "../math/util.hpp"

#include <mutex>
#include <chrono>
#include <string>
#include <iostream>
#include <map>
#include <cmath>
#include <vector>

namespace util {

	class Timer {
		private:
			static constexpr bool time = true;

			static constexpr int barWidth = 30;
			static constexpr int barPrecision = 3;
			static constexpr const char* barCharacters = " |#";//" |█";
			static std::map<std::string, double> timers;
			static std::vector<std::string> order;
			static uint64_t lastSummaryTime;
			static int checksSinceLastSummary; 
			static std::mutex mutex;

			uint64_t startTime;
			std::string label;
			bool active;

		public:
			static std::string lastSummary;

			Timer(const std::string& _label, bool _active = false) {
				active = _active || time;
				if (active) {
					label = _label;
					startTime = getTime();
				}
			}

			void end() {
				if (active) { std::unique_lock lock { mutex };
					double duration = (getTime() - startTime) / 1000.0;
					if (!timers.count(label)) {
						timers[label] = duration;
						order.push_back(label);
					} else
						timers[label] += duration;
					active = false;
				}
			}

			~Timer() {
				end();
			}
			
			// microseconds
			static uint64_t getTime() {
				return std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::high_resolution_clock::now().time_since_epoch()
				).count();
			}

			static double getMilliseconds() {
				return getTime() / 1000.0;
			}

			static double getSeconds() {
				return getMilliseconds() / 1000.0;
			}

			static std::string getSummaryString() { std::unique_lock lock { mutex };
				checksSinceLastSummary++;
				if (getTime() - lastSummaryTime < 100e3)
					return lastSummary;

				lastSummary = "================================ Timers ================================\n";

				int maxDuration = 0;
				int totalDuration = 0;
				
				for (const std::string& label : order) {
					double value = timers[label];
					maxDuration = std::max(maxDuration, (int)value);
					totalDuration += value;
				}

				for (const std::string& label : order) {
					double value = timers[label];
					float percent = value / maxDuration;
					
					std::string duration = leftPad(
						floatToString(math::roundTo(value / checksSinceLastSummary, 2)), 5
					) + " ms"; 	
					std::string percentage = leftPad(std::to_string((int)(value / totalDuration * 100)), 3) + "%";

					lastSummary += "[";
					float cutoff = percent * barWidth;
					for (int i = 0; i < barWidth; i++) {
						if (i == floor(cutoff) && i < cutoff) {
							lastSummary += barCharacters[(int)(barPrecision * (cutoff - i))];
						} else lastSummary += i < cutoff ? barCharacters[barPrecision - 1] : barCharacters[0];
					}

					lastSummary += "] | " + duration + " | " + percentage + " | " + label + "\n";
				}

				timers.clear();
				order.clear();
				
				checksSinceLastSummary = 0;
				lastSummaryTime = getTime();

				return lastSummary;
			}
	};

	std::map<std::string, double> Timer::timers { };
	std::string Timer::lastSummary = "";
	std::vector<std::string> Timer::order { };
	uint64_t Timer::lastSummaryTime = 0;
	int Timer::checksSinceLastSummary = 0;
	std::mutex Timer::mutex { };

	class PerformanceMonitor {
		private:
			std::vector<int> runningFPS;
			uint64_t lastTime;
			int fps;

		public:
			PerformanceMonitor() {
				lastTime = Timer::getTime();
			}

			PerformanceMonitor& update() {
				uint64_t currentTime = Timer::getTime();
				runningFPS.insert(runningFPS.begin(), 1e6 / (currentTime - lastTime));
				lastTime = currentTime;
				while (runningFPS.size() > 20)
					runningFPS.pop_back();

				int totalFPS = 0;
				for (int fps : runningFPS)
					totalFPS += fps;
				fps = totalFPS / runningFPS.size();
				return *this;
			}

			int getFPS() {
				return fps;
			}
	};
}