#pragma once

#include "gl.hpp"
#include "../math/util.hpp"
#include "../util/files.hpp"

#include <filesystem>
#include <fstream>

namespace gl {
	class Texture {
		private:
			static int nextUnit;

			GL& gl;

			void configureTexture() {
				gl.genTextures(1, &texture);
				gl.activeTexture(GL_TEXTURE0 + unit);
				gl.bindTexture(GL_TEXTURE_2D, texture);

				gl.texImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

				gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				setFiltering(true);
			}

		public:
			GLuint texture;
			GLenum internalFormat, format, type;
			int unit, width, height;

			Texture(GL& _gl, int _unit, int _width, int _height, GLenum _internalFormat = GL_RGBA, GLenum _format = GL_RGBA, GLenum _type = GL_UNSIGNED_BYTE) : gl(_gl) {
				unit = _unit;
				width = _width;
				height = _height;
				internalFormat = _internalFormat;
				format = _format;
				type = _type;

				configureTexture();
			}

			Texture(GL& _gl, int _unit, const std::string& bmpPath) : gl(_gl) {
				unit = _unit;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
				internalFormat = GL_RGBA;

				// read file
				std::string fileContent = util::readFile(bmpPath);

				std::unique_ptr<char[]> data { new char[fileContent.size()] };
				for (int i = 0; i < fileContent.size(); i++)
					data[i] = fileContent[i];

				void* pointer = data.get();
				#define READ_RESERVED(type) pointer = (void*)((type*)pointer + 1)
				#define READ_FIELD(type, name) type name = *(type*)pointer; READ_RESERVED(type);
				// #define READ_FIELD_P(type, name) READ_FIELD(type, name); PRINT(name)
				#define READ_ARRAY(type, count, name) type* name = (type*)pointer; pointer = (void*)((type*)pointer + count)

				// Bitmap File Header
				READ_ARRAY(char, 2, Signature);
				if (Signature[0] != 'B' || Signature[1] != 'M')
					throw std::runtime_error("Incorrect Signature");
				
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

				void* imageDataStart = data.get() + PixelDataOffset;
				auto alignPointer = [&]() {
					size_t align = sizeof(int);
					size_t relPointer = (size_t)pointer - (size_t)imageDataStart;
					if (relPointer != relPointer / align * align) {
						relPointer = (relPointer / align + 1) * align;
						pointer = (void*)(relPointer + (size_t)imageDataStart);
					}
				};

				size_t pixelInx = 0;

				if (compression == BI_RGB || compression == BI_BITFIELDS) {
					
					struct Color {
						unsigned char b, g, r;
					};
					
					std::unique_ptr<Color[]> imageData { new Color[width * height] };

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
								Color& c = imageData[pixelInx++];
								c.r = (Pixel & RedChannelMask) >> redChannelOffset;
								c.g = (Pixel & GreenChannelMask) >> greenChannelOffset;
								c.b = (Pixel & BlueChannelMask) >> blueChannelOffset;
							}

							alignPointer();
						}
					} else if (bpp == 24) {
							pointer = imageDataStart;
							for (int j = 0; j < height; j++) {
								// read scan line
								for (int i = 0; i < width; i++) {
									READ_FIELD(Color, Pixel);
									imageData[pixelInx++] = Pixel;
								}

								alignPointer();
							}

					} else if (bpp == 4) {
						std::unique_ptr<Color[]> palette { new Color[paletteSize] };
						for (int i = 0; i < paletteSize; i++) {
							READ_FIELD(Color, PaletteColor);
							READ_RESERVED(unsigned char);
							palette[i] = PaletteColor;
						}

						pointer = imageDataStart;
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

					}
				
					format = GL_BGR;
					configureTexture();
					set(imageData.get(), 1);
				}

				#undef READ_RESERVED
				#undef READ_FIELD
				#undef READ_ARRAY
			}

			Texture(Texture&& other) : gl(other.gl) {
				if (this != &other) {
					texture = other.texture;
					internalFormat = other.internalFormat;
					format = other.format;
					type = other.type;
					unit = other.unit;
					width = other.width;
					height = other.height;
					other.texture = 0;
				}
			}

			~Texture() {
				use();
				gl.deleteTextures(1, &texture);
			}

			void setFiltering(bool linear) {
				use();
				GLenum filter = linear ? GL_LINEAR : GL_NEAREST;
				gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
				gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);	
			}

			void bind(int newUnit) {
				unit = newUnit;
				use();
				gl.bindTexture(GL_TEXTURE_2D, newUnit);
			}

			void use() {
				gl.activeTexture(GL_TEXTURE0 + unit);
			}

			void set(void* data, int alignment = 4) {
				use();
				gl.pixelStorei(GL_UNPACK_ALIGNMENT, alignment);
				gl.texImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
			}

			void resize(int _width, int _height) {
				width = _width;
				height = _height;
				use();
				gl.texImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
			}

			static int next() {
				return nextUnit++;
			}
	};

	int Texture::nextUnit = 0;
}