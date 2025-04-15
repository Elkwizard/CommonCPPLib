#pragma once

#include "gl.hpp"
#include "color.hpp"

#include "../util/files.hpp"
#include "../util/string.hpp"
#include "../util/debug.hpp"

#include <fstream>
#include <unordered_map>
#include <map>
#include <regex>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace gl {
	class GLSL {
		private:
			GLsizei logBufferSize = 2048;
			std::unique_ptr<GLchar[]> logBuffer;
			GLint logLength;
			GLint program, vs, fs;
			GLuint vertexArray;

		private:
			using ErrorCallback = std::function<void(std::string, std::string)>;
			ErrorCallback onerror;
			std::string name;
			GL& gl;

			void handleLog(std::string type) {
				if (logLength > 0) {
					std::string msg = "";
					for (int i = 0; i < logLength; i++)
						msg += logBuffer[i];
					onerror(type, msg);
				}
			}

			std::string loadShaderSource(const std::string& src) {
				std::string baseSrc = util::directoryName(src);
				std::string source = util::normalizeLinebreaks(util::readFile(src));
				std::vector<std::string> lines = util::split<char>(source, "\n");
				std::string result = "";
				for (const std::string& line : lines) {
					if (line.starts_with("#include")) {
						std::string prefix = "#include \"";
						std::string suffix = "\"";
						std::string includeSrc = baseSrc + "/" + line.substr(
							prefix.size(),
							line.size() - prefix.size() - suffix.size()
						);
						std::string includedSource = loadShaderSource(includeSrc);
						result += includedSource + "\n";
					} else result += line + "\n";
				}
				return result;
			}

			std::pair<GLint, std::string> loadShader(GLenum type, std::string src) {
				GLint shader = gl.createShader(type);

				std::string shaderSource = loadShaderSource(src);

				GLsizei count = 1;
				const GLchar* string[1] { shaderSource.c_str() };
				const GLint length[1] { (GLint)shaderSource.length() };

				gl.shaderSource(shader, count, string, length);
				gl.compileShader(shader);
		

				gl.getShaderInfoLog(shader, logBufferSize, &logLength, logBuffer.get());		
				handleLog(type == GL_VERTEX_SHADER ? "VERTEX_SHADER" : "FRAGMENT_SHADER");
		
				return { shader, shaderSource };
			}

			struct TypeInfo {
				bool isInteger = false, isSigned = true, isTexture = false, isMatrix = false, isArray = false;
				int rows = 1, columns = 1, length = 1;
				std::string name;

				TypeInfo() { }

				TypeInfo(GLenum type, int _length, std::string _name) {
					name = _name;
					length = _length;
					
					switch (type) {
						case GL_FLOAT: break;
						case GL_FLOAT_VEC2: rows = 2; break;
						case GL_FLOAT_VEC3: rows = 3; break;
						case GL_FLOAT_VEC4: rows = 4; break;

						case GL_INT: isInteger = true; break;
						case GL_INT_VEC2: isInteger = true; rows = 2; break;
						case GL_INT_VEC3: isInteger = true; rows = 3; break;
						case GL_INT_VEC4: isInteger = true; rows = 4; break;

						case GL_BOOL: isInteger = true; break;
						case GL_BOOL_VEC2: isInteger = true; break;
						case GL_BOOL_VEC3: isInteger = true; break;
						case GL_BOOL_VEC4: isInteger = true; break;

						case GL_FLOAT_MAT2: rows = 2; columns = 2; break;
						case GL_FLOAT_MAT3: rows = 3; columns = 3; break;
						case GL_FLOAT_MAT4: rows = 4; columns = 4; break;

						case GL_FLOAT_MAT2x3: rows = 3; columns = 2; break;
						case GL_FLOAT_MAT2x4: rows = 4; columns = 2; break;
						case GL_FLOAT_MAT3x2: rows = 2; columns = 3; break;
						case GL_FLOAT_MAT3x4: rows = 4; columns = 3; break;
						case GL_FLOAT_MAT4x2: rows = 2; columns = 4; break;
						case GL_FLOAT_MAT4x3: rows = 3; columns = 4; break;

						case GL_INT_SAMPLER_1D:
						case GL_INT_SAMPLER_2D:
						case GL_INT_SAMPLER_3D:
						case GL_INT_SAMPLER_BUFFER:
						case GL_UNSIGNED_INT_SAMPLER_1D:
						case GL_UNSIGNED_INT_SAMPLER_2D:
						case GL_UNSIGNED_INT_SAMPLER_3D:
						case GL_UNSIGNED_INT_SAMPLER_BUFFER:
						case GL_SAMPLER_1D:
						case GL_SAMPLER_2D:
						case GL_SAMPLER_3D:
						case GL_SAMPLER_BUFFER: isInteger = true; isTexture = true; break;

						case GL_UNSIGNED_INT: isInteger = true; isSigned = false; break;
						case GL_UNSIGNED_INT_VEC2: isInteger = true; rows = 2; isSigned = false; break;
						case GL_UNSIGNED_INT_VEC3: isInteger = true; rows = 3; isSigned = false; break;
						case GL_UNSIGNED_INT_VEC4: isInteger = true; rows = 4; isSigned = false; break;
					}

					isArray = name.ends_with("[0]");
					isMatrix = columns > 1;
				}
			};

		public:
			class UniformNode {
				private:
					GLSL& glsl;
					GL& gl;
					std::unordered_map<std::string, UniformNode> members { };
					std::vector<std::string> memberNames { };
					TypeInfo info { };
					int size = 0;
					GLint location = -1;
					
					const void* set(const void* values, int count = -1) {
						if (members.size() > 0) {
							if (count == -1) count = memberNames.size();

							for (int i = 0; i < count; i++)
								values = members.at(memberNames[i]).set(values);
							return values;
						} else if (location != -1) {
							if (count == -1) count = info.length;

							int size = info.rows * info.columns;
							if (info.isMatrix) {
								using SetMatrixArray = std::function<void(GLint, GLint, GLboolean, GLfloat*)>;
								std::unordered_map<int, std::unordered_map<int, SetMatrixArray>> fnMap {
									{ 2, {
										{ 2, gl.uniformMatrix2fv },
										{ 3, gl.uniformMatrix2x3fv },
										{ 4, gl.uniformMatrix2x4fv }
									} },
									{ 3, {
										{ 2, gl.uniformMatrix3x2fv },
										{ 3, gl.uniformMatrix3fv },
										{ 4, gl.uniformMatrix3x4fv }
									} },
									{ 4, { 
										{ 2, gl.uniformMatrix4x2fv },
										{ 3, gl.uniformMatrix4x3fv },
										{ 4, gl.uniformMatrix4fv }
									} }
								};
								SetMatrixArray fn = fnMap.at(info.columns).at(info.rows);
								fn(location, (GLsizei)count, false, (GLfloat*)values);
							} else {
								if (info.isInteger) {
									using SetVectorArray = std::function<void(GLint, GLint, GLint*)>;
									std::unordered_map<int, SetVectorArray> fnMap {
										{ 1, gl.uniform1iv },
										{ 2, gl.uniform2iv },
										{ 3, gl.uniform3iv },
										{ 4, gl.uniform4iv }
									};
									SetVectorArray fn = fnMap.at(info.rows);
									fn(location, (GLsizei)count, (GLint*)values);
								} else {
									using SetVectorArray = std::function<void(GLint, GLint, GLfloat*)>;
									std::unordered_map<int, SetVectorArray> fnMap {
										{ 1, gl.uniform1fv },
										{ 2, gl.uniform2fv },
										{ 3, gl.uniform3fv },
										{ 4, gl.uniform4fv }
									};
									SetVectorArray fn = fnMap.at(info.rows);
									fn(location, (GLsizei)count, (GLfloat*)values);
								}
							}
							
							return (void*)((GLfloat*)values + count * size);
						} else return values;
					}

				public:
					UniformNode(GLSL& _glsl) : gl(_glsl.gl), glsl(_glsl) { }
					
					UniformNode(GLSL& _glsl, GLenum type, int length, std::string name, GLint _location) : gl(_glsl.gl), glsl(_glsl) {
						info = { type, length, name };
						location = _location;
						size = info.rows * info.columns * sizeof(GLfloat) * info.length;
					}

					bool hasMember(std::string name) {
						return members.count(name);
					}

					void setMember(std::string name, UniformNode node) {
						size += node.size;
						members.emplace(name, node);

						bool array = !std::any_of(name.begin(), name.end(), [](const char& ch) {
							return !isdigit(ch);
						});
						
						int index = 0;
						while (
							index < memberNames.size() &&
							(array ? std::stoi(memberNames[index]) < std::stoi(name) : memberNames[index] < name)
						) index++;
						memberNames.insert(memberNames.begin() + index, name);
					}

					UniformNode& operator [](std::string name) {
						if (!members.count(name)) {
							members.emplace(name, glsl);
							// glsl.onerror("UNIFORM_SET", "Uniform '" + name + "' doesn't exist");
						}
						return members.at(name);
					}

					UniformNode& operator [](size_t index) {
						return (*this)[std::to_string(index)];
					}
					
					void operator =(const void* values) {
						set(values);
					}

					template <typename T>
					void operator =(std::vector<T> array) {
						if ((memberNames.size() && array.size() > memberNames.size()) || (location != -1 && array.size() > info.length))
							glsl.onerror("UNIFORM_SET", "Too many array elements");
						set(&array[0], array.size());
					}

					void operator =(std::pair<void*, int> array) {
						set(array.first, array.second);
					}

					void operator =(GLint value) {
						set(&value);
					}

					void operator =(GLuint value) {
						set(&value);
					}

					void operator =(GLfloat value) {
						set(&value);
					}

					void operator =(GLboolean value) {
						int padded = value;
						set(&padded);
					}
					
					void operator =(const Color& value) {
						float rgba[] { value.r, value.g, value.b, value.a };
						set(rgba);
					}
			};

			class Attribute {
				public:
					std::string name = "";
					TypeInfo info { };
					int divisor = -1, location = -1;
					bool isFiller = false;
					bool enabled = false;

					Attribute() { }
					Attribute(GLenum _type, int _length, std::string _name, int _location) {
						info = { _type, _length, _name };
						name = _name;
						location = _location;
					}
					Attribute(int rows) {
						info.rows = rows;
						info.columns = 1;
						isFiller = true;
					}

					bool operator ==(Attribute b) {
						return name == b.name;
					}
			};

			class Divisor {
				private:
					GL& gl;
					GLuint buffer;

				public:
					int divisor = -1;
					std::vector<Attribute> attributes { };
					int stride = 0;

					Divisor(GL& _gl, int _divisor) : gl(_gl) {
						divisor = _divisor;
					}

					void operator =(const Divisor& other) {
						divisor = other.divisor;
						stride = other.stride;
						attributes = other.attributes;
					}

					void operator =(GLuint _buffer) {
						buffer = _buffer;
						gl.bindBuffer(GL_ARRAY_BUFFER, buffer);

						GLfloat* offset = 0;
						for (int i = 0; i < attributes.size(); i++) {
							Attribute& attribute = attributes[i];
							if (attribute.isFiller) {
								offset += attribute.info.rows * attribute.info.columns;
							} else {
								bool enablingNeeded = !attribute.enabled;
								if (enablingNeeded) attribute.enabled = true;
								for (int j = 0; j < attribute.info.columns; j++) {
									int pointer = attribute.location + j;
									gl.vertexAttribPointer(pointer, attribute.info.rows, GL_FLOAT, false, stride, (void*)offset);
									gl.vertexAttribDivisor(pointer, divisor);
									if (enablingNeeded) gl.enableVertexAttribArray(pointer);
									offset += attribute.info.rows;
								}
							}
						}
					}
					
					template <typename T>
					void operator =(const std::vector<T>& list) {
						gl.bindBuffer(GL_ARRAY_BUFFER, buffer);
						gl.bufferData(GL_ARRAY_BUFFER, list.size() * sizeof(T), &list[0], GL_DYNAMIC_DRAW);
						for (int i = 0; i < list.size() * sizeof(T) / sizeof(GLfloat); i++) {
							float value = ((GLfloat*)&list[0])[i];
							// std::cout << "floats[" << i << "] = " << value << std::endl;
						}
						*this = buffer;
					}
			};

			class DivisorMap : public std::unordered_map<int, Divisor> {
				private:
					GLSL& glsl;

				public:
					DivisorMap(GLSL& _glsl) : glsl(_glsl) { }
					Divisor& operator [](const int& key) {
						if (!count(key))
							glsl.onerror("ATTRIBUTE_SET", "No attributes exist with divisor " + std::to_string(key));
						return at(key);
					}

					template <int N>
					void operator =(GLuint (&buffers)[N]) {
						for (int i = 0; i < N; i++)
							(*this)[i] = buffers[i];
					}

					void operator =(std::vector<GLuint> buffers) {
						for (int i = 0; i < buffers.size(); i++)
							(*this)[i] = buffers[i];
					}
			};

			class UniformBuffer {
				private:
					GL& gl;
					GLSL& glsl;
					std::string name;
					GLint index, binding;
				
				public:
					UniformBuffer(GLSL& _glsl) : glsl(_glsl), gl(_glsl.gl){
						name = "";
						index = -1;
					}

					UniformBuffer(GLSL& _glsl, std::string _name, GLint _index) : glsl(_glsl), gl(_glsl.gl) {
						name = _name;
						index = _index;
					}

					void bind(GLuint _binding) {
						binding = _binding;
						gl.uniformBlockBinding(glsl.program, index, binding);
					}

					void operator =(GLuint buffer) {
						gl.bindBufferBase(GL_UNIFORM_BUFFER, binding, buffer);
					}
			};

			class UniformBufferMap : public std::unordered_map<std::string, UniformBuffer> {
				private:
					GLSL& glsl;

				public:
					UniformBufferMap(GLSL& _glsl) : glsl(_glsl) { }
					UniformBuffer& operator [](std::string key) {
						if (!count(key)) {
							emplace(key, UniformBuffer(glsl));
							//return { };
							// glsl.onerror("UNIFORM_BUFFER_SET", "No uniform buffers exist with name " + key);
						}
						return at(key);
					}
			};

		public:
			class LayoutSegment {
				public:
					std::string name;
					int size;
					bool isFiller;

					LayoutSegment(std::string _name) {
						name = _name;
						isFiller = false;
					}
					LayoutSegment(int _size) {
						size = _size;
						isFiller = true;
					}
			};

			UniformNode uniforms;
			DivisorMap divisors;
			UniformBufferMap uniformBuffers;
			std::unordered_map<std::string, Attribute> attributes { };
			GLSL(GL& _gl, std::string _name, ErrorCallback _onerror = nullptr)
			: gl(_gl), uniforms({ *this }), divisors({ *this }) , uniformBuffers({ *this }) {
				onerror = _onerror == nullptr ? [=, this](std::string type, std::string msg) {
					throw std::runtime_error(("[" + name + "] " + type + " error: " + msg).c_str());
				} : _onerror;
				name = _name;

				logBuffer = std::make_unique<GLchar[]>(logBufferSize);
				
				{ // create program
					program = gl.createProgram();
				
					auto vShader = loadShader(GL_VERTEX_SHADER, name + "/vert.glsl");
					auto fShader = loadShader(GL_FRAGMENT_SHADER, name + "/frag.glsl");
					vs = vShader.first;
					fs = fShader.first;
					
					gl.attachShader(program, vs);
					gl.attachShader(program, fs);

					gl.linkProgram(program);

					gl.getProgramInfoLog(program, logBufferSize, &logLength, logBuffer.get());
					handleLog("LINKING");
				};

				{ // initialize
					GLint originalProgram;
					gl.getIntegerv(GL_CURRENT_PROGRAM, &originalProgram);

					gl.useProgram(program);


					{ // uniforms
						GLint maxUniformNameLength;
						gl.getProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
						
						GLint uniformCount;
						gl.getProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

						// uniformValues = {};

						int nextTextureUnit = 0;
						GLint maxTextureUnits;
						gl.getIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

						for (int i = 0; i < uniformCount; i++) {
							GLint length;
							GLenum type;
							std::string name = "";
							
							{ // get data
								GLchar* nameChars = new GLchar[maxUniformNameLength];
								GLsizei nameLength;
								gl.getActiveUniform(program, (GLuint)i, maxUniformNameLength, &nameLength, &length, &type, nameChars);
								for (int i = 0; i < nameLength; i++)
									name += nameChars[i];

								delete[] nameChars;
							}

							std::regex rgx { "\\[(\\d+)\\]" };
							std::string processedName = std::regex_replace(name, rgx, ".$1");
							auto split = [](std::string str, std::string delim) {
								std::vector<std::string> pieces { };
								
								while (true) {
									int inx = str.find(delim);
									if (inx == -1) break;
									pieces.push_back(str.substr(0, inx));
									str = str.substr(inx + delim.length());
								}

								pieces.push_back(str);
								
								return pieces;
							};
							std::vector<std::string> propertyPath = split(processedName, ".");
							if (propertyPath[propertyPath.size() - 1] == "0") propertyPath.pop_back();

							GLint location = gl.getUniformLocation(program, name.c_str());

							UniformNode* currentStruct = &uniforms;
							for (int i = 0; i < propertyPath.size(); i++) {
								std::string property = propertyPath[i];
								bool last = i == propertyPath.size() - 1;

								if (last) {
									currentStruct->setMember(property, { *this, type, length, name, location });
								} else {
									if (!currentStruct->hasMember(property)) currentStruct->setMember(property, { *this });
									currentStruct = &((*currentStruct)[property]);
								}
							}
						}
					};

					{ // uniform blocks
					
						GLint maxUniformNameLength;
						gl.getProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxUniformNameLength);

						GLint uniformCount;
						gl.getProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &uniformCount);

						for (int i = 0; i < uniformCount; i++) {
							GLint length;
							GLenum type;
							std::string name = "";
							
							{ // get data
								GLchar* nameChars = new GLchar[maxUniformNameLength];
								GLsizei nameLength;
								gl.getActiveUniformBlockName(program, i, maxUniformNameLength, &nameLength, nameChars);
								for (int i = 0; i < nameLength; i++)
									name += nameChars[i];

								delete[] nameChars;
							};

							uniformBuffers.emplace(name, UniformBuffer(*this, name, i));
						}
					};

					{ // attributes
						gl.createVertexArrays(1, &vertexArray);

						GLint attributeCount;
						gl.getProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);
					
						GLint maxAttribNameLength;
						gl.getProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribNameLength);

						for (int i = 0; i < attributeCount; i++) {
							GLint length;
							GLenum type;
							std::string name = "";
							
							{ // get data
								std::unique_ptr<GLchar[]> nameChars { new GLchar[maxAttribNameLength] };
								GLsizei nameLength;
								gl.getActiveAttrib(program, (GLuint)i, maxAttribNameLength, &nameLength, &length, &type, nameChars.get());
								for (int i = 0; i < nameLength; i++)
									name += nameChars[i];
							}

							if (name.starts_with("gl_")) // built-in
								continue;

							GLint location = gl.getAttribLocation(program, name.c_str());
							attributes[name] = { type, length, name, location };
							
							setDivisor(name, 0);
						}
					};

					gl.useProgram(originalProgram);
				};
			}

			~GLSL() {
				gl.detachShader(program, vs);
				gl.detachShader(program, fs);
				gl.deleteShader(vs);
				gl.deleteShader(fs);
				gl.deleteProgram(program);
			}

			void use() {
				gl.useProgram(program);
				gl.bindVertexArray(vertexArray);
			}
			
			void layoutAttributes(std::vector<LayoutSegment> layout, int divisor = 0) {
				std::vector<Attribute> currentAttrList = divisors[divisor].attributes;
				std::unordered_map<std::string, Attribute> currentAttrs = { };
				for (int i = 0; i < currentAttrList.size(); i++) {
					auto& attribute = currentAttrList[i];
					if (!attribute.isFiller)
						currentAttrs[attribute.name] = attribute;
				}

				std::vector<Attribute> attributes { };
				int stride = 0;
				for (int i = 0; i < layout.size(); i++) {
					LayoutSegment segment = layout[i];
					Attribute attr = segment.isFiller ? Attribute(segment.size) : currentAttrs[segment.name];
					attributes.push_back(attr);
					
					stride += attr.info.rows * attr.info.columns * sizeof(GLfloat);
				}

				divisors[divisor].attributes = attributes;
				divisors[divisor].stride = stride;
			}

			void setDivisor(std::string name, int divisor) {
				use();
				if (attributes.count(name)) {
					Attribute& attr = attributes[name];

					if (divisors.count(attr.divisor)) { // remove from previous
						auto& prev = divisors[attr.divisor];
						
						int index = (int)(std::find(
							prev.attributes.begin(),
							prev.attributes.end(),
							attr
						) - prev.attributes.begin());

						prev.attributes.erase(prev.attributes.begin() + index);
						prev.stride -= attr.info.rows * attr.info.columns * sizeof(GLfloat);
					}

					if (!divisors.count(divisor)) // create divisor
						divisors.emplace(divisor, Divisor(gl, divisor));

					Divisor& current = divisors[divisor];

					current.attributes.push_back(attr);
					std::sort(current.attributes.begin(), current.attributes.end(), [](const Attribute& a, const Attribute& b) {
						return a.location < b.location;
					});
					current.stride += attr.info.rows * attr.info.columns * sizeof(GLfloat);

					attr.divisor = divisor;
				} else onerror("DIVISOR_SET", "Vertex attribute '" + name + "' doesn't exist");
			}
	};
}