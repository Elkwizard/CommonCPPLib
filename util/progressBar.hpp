#pragma once

#include "math.hpp"

#include <iostream>
#include <string>

namespace util {
	class ProgressBar {
		private:
			int counter, total;
			int width;
			std::string name;
			std::string lastDisplayed = "";

		public:

			ProgressBar(int _total, const std::string& _name = "", int _width = 30) {
				total = total;
				counter = 0;
				width = _width;
				name = _name;
			}	

			void setProgress(int progress) {
				counter = progress;
			}

			void increment() {
				counter++;
			}

			void display() {
				if (counter > total)
					return;

				std::string toDisplay = "[";
				int filled = clamp((float)counter / total, 0.0f, 1.0f) * width;
				for (int i = 0; i < filled; i++)
					toDisplay += "#";
				for (int i = filled; i < width; i++)
					toDisplay += " ";
				toDisplay += "]";
				if (!name.empty())
					toDisplay += " " + name;
				std::cout << "\r" << std::string(lastDisplayed.size(), ' ') << std::flush;	
				std::cout << "\r" << toDisplay << std::flush;
				lastDisplayed = toDisplay;

				if (counter == total)
					std::cout << std::endl;
			}
	};
}