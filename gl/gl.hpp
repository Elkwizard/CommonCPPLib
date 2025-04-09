#pragma once

#include "window.hpp"
#include "loader.hpp"
#include <iostream>

#pragma comment(lib, "Gdi32")

#define CHECK std::cout << __FILE__ ": " << __LINE__ << std::endl;

namespace gl {
	using namespace window;

	class GL : public LoadedGL {
		private:
			HDC dc;
			HGLRC ctx;
			std::unique_ptr<Callback> callback;

		public:
			Window& w;
			
			GL(Window& _w, bool debug = true) : w(_w) {
				dc = GetDC(w.handle);

				PIXELFORMATDESCRIPTOR pfd = {
					sizeof(PIXELFORMATDESCRIPTOR),
					1,
					PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
					PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
					32,                   // Colordepth of the framebuffer.
					0, 0, 0, 0, 0, 0,
					0,
					0,
					0,
					0, 0, 0, 0,
					24,                   // Number of bits for the depthbuffer
					8,                    // Number of bits for the stencilbuffer
					0,                    // Number of Aux buffers in the framebuffer.
					PFD_MAIN_PLANE,
					0,
					0, 0, 0
				};

				int pixelFormat = ChoosePixelFormat(dc, &pfd);
				SetPixelFormat(dc, pixelFormat, &pfd);

				HGLRC dummy = wglCreateContext(dc);
				wglMakeCurrent(dc, dummy);

				PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

				wglMakeCurrent(dc, NULL);
				wglDeleteContext(dummy);

				int attribs[] {
					WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
					WGL_CONTEXT_MINOR_VERSION_ARB, 3,
					WGL_CONTEXT_FLAGS_ARB, debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
					WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,// | WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
					0
				};

				ctx = wglCreateContextAttribsARB(dc, NULL, attribs);
				
				wglMakeCurrent(dc, ctx);

				loadProcedures();

				if (debug) {
					enable(GL_DEBUG_OUTPUT);
					debugMessageCallback(errorCallback, NULL);
				}
				
				callback = std::make_unique<Callback>(w, w.timerEvent, [&] {
					swapBuffers();
				});
			}
			
			~GL() {
				wglDeleteContext(ctx);
				ReleaseDC(w.handle, dc);
			}

			void swapBuffers() {
				wglSwapLayerBuffers(dc, WGL_SWAP_MAIN_PLANE);
			}

			std::string getErrorString() {
				switch (getError()) {
					case GL_INVALID_ENUM:
						return "GL_INVALID_ENUM";
					case GL_INVALID_VALUE:
						return "GL_INVALID_VALUE";
					case GL_INVALID_OPERATION:
						return "GL_INVALID_OPERATION";
					case GL_STACK_OVERFLOW:
						return "GL_STACK_OVERFLOW";
					case GL_STACK_UNDERFLOW:
						return "GL_STACK_UNDERFLOW";
					case GL_OUT_OF_MEMORY:
						return "GL_OUT_OF_MEMORY";
					case GL_INVALID_FRAMEBUFFER_OPERATION:
						return "GL_INVALID_FRAMEBUFFER_OPERATION";
					case GL_CONTEXT_LOST:
						return "GL_CONTEXT_LOST";
					default:
						return "GL_NO_ERROR";
				}
			}

			static void APIENTRY errorCallback(
				GLenum source, GLenum type, GLuint id, GLenum severity,
				GLsizei length, const GLchar *message, const void *userParam
			) {
				std::string sourceStr = "UNKNOWN";
				switch (source) {
					case GL_DEBUG_SOURCE_API:
						sourceStr = "API";
						break;
					case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
						sourceStr = "Window System";
						break;
					case GL_DEBUG_SOURCE_SHADER_COMPILER:
						sourceStr = "Shader Compiler";
						break;
					case GL_DEBUG_SOURCE_THIRD_PARTY:
						sourceStr = "Third Party";
						break;
					case GL_DEBUG_SOURCE_APPLICATION:
						sourceStr = "Application";
						break;
					case GL_DEBUG_SOURCE_OTHER:
						sourceStr = "Other";
						break;
				}

				std::string messageStr = sourceStr + " error: ";
				messageStr += std::string(message, length);

				if (messageStr.find("warning") == -1 && messageStr.find("info") == -1)
					throw std::runtime_error(messageStr);
				// else std::cout << messageStr << std::endl;
				
			}
	};
};