#pragma once

#include <memory>
#include <string>

namespace gl {
	class Bitmap {
		public:
			struct RGB {
				unsigned char r, g, b, a;
			};

		private:
			std::unique_ptr<RGB[]> pixels;
			size_t width;
			size_t height;

		public:
			Bitmap(const std::string& path);
			size_t getWidth() const;
			size_t getHeight() const;
			size_t getSize() const;
			RGB* const getPixels() const;
	};
}