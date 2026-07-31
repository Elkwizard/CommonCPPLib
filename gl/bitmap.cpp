#include "bitmap.hpp"
#include "../util/windows.hpp"
#include "../util/files.hpp"
#include "../math/util.hpp"
#include <stdexcept>
#include <iostream>

namespace gl {
	Bitmap::Bitmap(const std::string& path) {
		std::string fileContent = util::readFile(path);

		std::unique_ptr<char[]> data { new char[fileContent.size()] };
		for (int i = 0; i < fileContent.size(); i++)
			data[i] = fileContent[i];

		void* pointer = data.get();
		#define READ_RESERVED(type) pointer = (void*)((type*)pointer + 1)
		#define READ_FIELD(type, name) type name = *(type*)pointer; READ_RESERVED(type);
		#define READ_ARRAY(type, count, name) type* name = (type*)pointer; pointer = (void*)((type*)pointer + count)

		// Bitmap File Header
		READ_ARRAY(char, 2, Signature);
		std::cout << Signature[0] << Signature[1] << std::endl;
		if (Signature[0] != 'B' || Signature[1] != 'M')
			throw std::runtime_error("Incorrect bitmap signature");
		
		READ_FIELD(int, FileSize);
		READ_RESERVED(short);
		READ_RESERVED(short);
		READ_FIELD(int, PixelDataOffset);

		// DIB Header
		READ_FIELD(BITMAPINFOHEADER, Header);
		// move to end of header, skipping unknown fields
		pointer = (void*)((size_t)pointer + (Header.biSize - sizeof(BITMAPINFOHEADER)));

		width = Header.biWidth;
		height = Header.biHeight;
		short bpp = Header.biBitCount;
		int compression = Header.biCompression;
		int paletteSize = Header.biClrUsed ? Header.biClrUsed : 1 << bpp;

		void* pixelsStart = data.get() + PixelDataOffset;
		auto alignPointer = [&]() {
			size_t align = sizeof(int);
			size_t relPointer = (size_t)pointer - (size_t)pixelsStart;
			if (relPointer != relPointer / align * align) {
				relPointer = (relPointer / align + 1) * align;
				pointer = (void*)(relPointer + (size_t)pixelsStart);
			}
		};

		size_t pixelInx = 0;

		if (!(compression == BI_RGB || compression == BI_BITFIELDS))
			throw std::runtime_error("Unsupported bitmap compression");

		struct BGR {
			unsigned char b, g, r;
		};

		size_t totalPixels = width * height;
		std::unique_ptr<BGR[]> imageData { new BGR[totalPixels] };

		if (bpp == 32) {
			READ_FIELD(unsigned int, RedChannelMask);
			READ_FIELD(unsigned int, GreenChannelMask);
			READ_FIELD(unsigned int, BlueChannelMask);
			
			int redChannelOffset = math::firstBitIndex(RedChannelMask);
			int greenChannelOffset = math::firstBitIndex(GreenChannelMask);
			int blueChannelOffset = math::firstBitIndex(BlueChannelMask);

			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					READ_FIELD(unsigned int, Pixel);
					BGR& c = imageData[pixelInx++];
					c.r = (Pixel & RedChannelMask) >> redChannelOffset;
					c.g = (Pixel & GreenChannelMask) >> greenChannelOffset;
					c.b = (Pixel & BlueChannelMask) >> blueChannelOffset;
				}

				alignPointer();
			}
		} else if (bpp == 24) {
				pointer = pixelsStart;
				for (int j = 0; j < height; j++) {
					// read scan line
					for (int i = 0; i < width; i++) {
						READ_FIELD(BGR, Pixel);
						imageData[pixelInx++] = Pixel;
					}

					alignPointer();
				}

		} else if (bpp == 4) {
			std::unique_ptr<BGR[]> palette { new BGR[paletteSize] };
			for (int i = 0; i < paletteSize; i++) {
				READ_FIELD(BGR, PaletteColor);
				READ_RESERVED(unsigned char);
				palette[i] = PaletteColor;
			}

			pointer = pixelsStart;
			for (int j = 0; j < height; j++) {
				// read scan line
				for (int i = 0; i < width; i += 2) {
					READ_FIELD(unsigned char, PaletteIndices);
					imageData[pixelInx++] = palette[PaletteIndices >> 4];
					if (i + 1 < width) 
						imageData[pixelInx++] = palette[PaletteIndices & 0b1111];
				}

				alignPointer();
			}
		} else {
			throw std::runtime_error("Unsupported bitmap bits-per-pixel");
		}

		pixels = std::make_unique<RGB[]>(totalPixels);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int srcIndex = width * y + x;
				int dstIndex = width * (height - y - 1) + x;
				BGR data = imageData[srcIndex];
				pixels[dstIndex] = { data.r, data.g, data.b, 255 };
			}
		}

		#undef READ_RESERVED
		#undef READ_FIELD
		#undef READ_ARRAY
	}

	size_t Bitmap::getWidth() const {
		return width;
	}

	size_t Bitmap::getHeight() const {
		return height;
	}

	size_t Bitmap::getSize() const {
		return width * height * sizeof(pixels[0]);
	}

	Bitmap::RGB* const Bitmap::getPixels() const {
		return pixels.get();
	}
}