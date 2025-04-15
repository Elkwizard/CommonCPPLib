#pragma once

#include "gl.hpp"
#include "color.hpp"
#include "glsl.hpp"
#include "texture.hpp"

namespace gl {
	template <typename T>
	concept IndexablePointLike = requires (T t) {
		{t[0]} -> std::convertible_to<float>;
		{t[1]} -> std::convertible_to<float>;
	};
	template <typename T>
	concept PropertyPointLike = requires (T t) {
		{t.x} -> std::convertible_to<float>;
		{t.y} -> std::convertible_to<float>;
	};
	template <typename T>
	concept PointLike = IndexablePointLike<T> || PropertyPointLike<T>;

	class Font {
		private:	
			std::unique_ptr<Texture> atlas;
			
		public:
			int charWidth, charHeight;
			std::string charset;

			Font(GL& gl, const std::string& path, int _charWidth, int _charHeight, int unit = 0) {
				charWidth = _charWidth;
				charHeight = _charHeight;
				std::string fontPath = path + "/" + std::to_string(charHeight) + "by" + std::to_string(charWidth);
				atlas = std::make_unique<Texture>(gl, unit, fontPath + "/font.bmp");
				charset = util::readFile(fontPath + "/charset.txt");
				atlas->setFiltering(false);
			}

			void setUnit(int unit) {
				atlas->bind(unit);
			}

			int getUnit() {
				return atlas->unit;
			}
	};

	class Context2D {
		public:
			struct Instance {
				float transform[6];
				FullColor color;
				float flags;
				float lineWidth;
			};

		private:
			#include "2d/shader/masks.glsl"

			GL& gl;
			std::unique_ptr<window::EventHandler> resizeHandler;
			std::unique_ptr<GLSL> shader;
			std::unique_ptr<Font> font;
			GLuint vertexBuffer, instanceBuffer;

			std::vector<Instance> instances;
			
			FullColor currentColor;
			int currentStyle;
			float currentLineWidth, currentLineRadius;

			template <IndexablePointLike T>
			float x(const T& point) { return point[0]; }
			template <IndexablePointLike T>
			float y(const T& point) { return point[1]; }
			
			template <PropertyPointLike T>
			float x(const T& point) { return point.x; }
			template <PropertyPointLike T>
			float y(const T& point) { return point.y; }

			void addInstance(
				float a, float b, float tx,
				float c, float d, float ty,
				int flags
			) {
				instances.push_back(Instance(
					{ a, c, b, d, tx, ty },
					currentColor,
					flags | (currentStyle << STYLE_OFFSET),
					currentLineWidth
				));
			}

			template <PointLike T>
			void lines(const std::vector<T>& points, bool closed) {
				for (int i = 1; i < points.size(); i++) {
					const T& a = points[i - 1];
					const T& b = points[i];
					line(a, b);
				}

				if (closed)
					line(points[points.size() - 1], points[0]);
			}

		public:
			Context2D(GL& _gl) : gl(_gl) {
				resizeHandler = std::make_unique<window::EventHandler>(gl.window.onResize, [&]() {
					gl.viewport(0, 0, gl.window.client.width, gl.window.client.height);
				});

				std::string localPath = util::directoryName(__FILE__) + "/2d";
				shader = std::make_unique<GLSL>(gl, localPath + "/shader", [](const std::string& type, const std::string& msg) {
					std::cout << "GLSL Error (" << type << "): \e[31m" << msg << "\e[0m";
				});
				
				shader->setDivisor("vertexPosition", 0);
				shader->setDivisor("instanceTransform", 1);
				shader->setDivisor("instanceColor", 1);
				shader->setDivisor("instanceFlags", 1);
				shader->setDivisor("instanceLineWidth", 1);

				{ // vertex data
					float vertexData[] {
						0, 0,
						0, 1,
						1, 0,
						1, 0,
						0, 1,
						1, 1
					};

					gl.genBuffers(1, &vertexBuffer);
					gl.bindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
					gl.bufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
					shader->divisors[0] = vertexBuffer;
				}

				{ // instance data
					gl.genBuffers(1, &instanceBuffer);
					gl.bindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
					shader->divisors[1] = instanceBuffer;
				}
				
				{ // text
					font = std::make_unique<Font>(gl, localPath, 5, 15, Texture::next());
					shader->uniforms["fontAtlas"] = font->getUnit();
					shader->uniforms["charWidth"] = font->charWidth;
				}
			}

			Context2D& stroke(const Color& color, float lineWidth = 1.0) {
				currentColor = color;
				currentLineWidth = lineWidth;
				currentLineRadius = currentLineWidth * 0.5;
				currentStyle = STROKE;
				return *this;
			}

			Context2D& draw(const Color& color) {
				currentColor = color;
				currentStyle = FILL;
				return *this;
			}

			void rect(float x, float y, float w, float h) {
				if (currentStyle == STROKE) {
					x -= currentLineRadius;
					y -= currentLineRadius;
					w += currentLineWidth;
					h += currentLineWidth;
				}

				addInstance(
					w, 0, x,
					0, h, y,
					RECTANGLE
				);
			}

			template <PointLike T>
			void rect(const T& min, const T& max) {
				rect(x(min), y(min), x(max) - x(min), y(max) - y(min));
			}

			void circle(float x, float y, float r) {
				if (currentStyle == STROKE)
					r += currentLineRadius;
				
				float w = r * 2;

				addInstance(
					w, 0, x - r,
					0, w, y - r,
					CIRCLE
				);
			}

			template <PointLike T>
			void circle(const T& p, float r) {
				circle(x(p), y(p), r);
			}

			template <PointLike T>
			void shape(const std::vector<T>& vertices) {
				if (currentStyle == STROKE) {
					lines(vertices, true);
				} else {
					for (int i = 2; i < vertices.size(); i++) {
						const T& a = vertices[0];
						const T& b = vertices[i - 1];
						const T& c = vertices[i];
						
						addInstance(
							x(b) - x(a), x(c) - x(a), x(a),
							y(b) - y(a), y(c) - y(a), y(a),
							TRIANGLE
						);
					}
				}
			}

			template <PointLike T>
			void connector(const std::vector<T>& points) {
				lines(points, false);
			}

			void line(float x1, float y1, float x2, float y2) {
				float vx = x2 - x1;
				float vy = y2 - y1;
				float scale = currentLineWidth / std::hypot(vx, vy);
				float nx = -vy * scale;
				float ny = vx * scale;

				addInstance(
					nx, vx, x1 - nx * 0.5,
					ny, vy, y1 - ny * 0.5,
					RECTANGLE
				);
			}

			template <PointLike T>
			void line(const T& a, const T& b) {
				line(x(a), y(a), x(b), y(b));
			}

			void arrow(float ox, float oy, float dx, float dy) {
				float ex = ox + dx;
				float ey = oy + dy;
				line(ox, oy, ex, ey);
				float arrowSize = currentLineWidth * 5.0;
				float scale = arrowSize / std::hypot(dx, dy);
				float nx = -dy * scale;
				float ny = dx * scale;
				float bx = ex - dx * scale * 2;
				float by = ey - dy * scale * 2;
				line(bx + nx, by + ny, ex, ey);
				line(bx - nx, by - ny, ex, ey);
			}

			template <PointLike T>
			void arrow(const T& o, const T& v) {
				arrow(x(o), y(o), x(v), y(v));
			}

			void text(float fontSize, const std::string& str, float x, float y) {
				float baseX = x;

				float h = fontSize;
				float w = font->charWidth * h / font->charHeight;
				float padding = w * 0.1;

				for (char c : str) {
					if (c == '\n') {
						y += h;
						x = baseX;
					} else {
						if (c == '\t') {
							x += w * 3;
						} else if (c != ' ') {
							int index = font->charset.find(c);
							addInstance(
								w, 0, x,
								0, h, y,
								TEXT | (index << CHAR_OFFSET)
							);
						}

						x += w + padding;
					}
				}
			}

			template <PointLike T>
			void text(float fontSize, const std::string& str, const T& location) {
				text(fontSize, str, x(location), y(location));
			}

			void flush() {
				shader->use();
				float resolution[] { (float)gl.window.client.width, (float)gl.window.client.height };
				shader->uniforms["resolution"] = resolution;

				gl.enable(GL_BLEND);
				gl.blendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				
				shader->divisors[1] = instances;
				gl.drawArraysInstanced(GL_TRIANGLES, 0, 6, instances.size());
				
				instances.clear();
			}

			void clear() {
				flush();
				gl.clearColor(0, 0, 0, 0);
				gl.clear(GL_COLOR_BUFFER_BIT);
			}
	};
};