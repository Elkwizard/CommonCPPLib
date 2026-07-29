#pragma once

#include "gl.hpp"
#include "bitmap.hpp"
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

				Bitmap bitmap { bmpPath };
				width = bitmap.getWidth();
				height = bitmap.getHeight();
				configureTexture();
				set(bitmap.getPixels(), 1);
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