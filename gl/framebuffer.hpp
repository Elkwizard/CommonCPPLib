#pragma once

#include "gl.hpp"
#include "texture.hpp"

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>

namespace gl {

	class Framebuffer {
		private:
			GL& gl;
			std::vector<GLenum> attachments { };
			std::unordered_map<int, int> attachmentMap { };
			int highestAttachment = 0;
		
		public:
			GLuint fbo, depthBuffer = -1;
			std::vector<std::unique_ptr<Texture>> textures { };
			int width = 0, height = 0;

			Framebuffer(GL& _gl, int _width, int _height) : gl(_gl) {
				width = _width;
				height = _height;

				gl.genFramebuffers(1, &fbo);
				gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);
			}

			~Framebuffer() {
				gl.deleteFramebuffers(1, &fbo);
			}

			void addTexture(int unit, int attachmentOffset = 0, GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE) {
				if (attachmentOffset > highestAttachment) highestAttachment = attachmentOffset;
				textures.push_back(std::make_unique<Texture>(gl, unit, width, height, internalFormat, format, type));
				attachmentMap[unit] = attachmentOffset;
				std::sort(textures.begin(), textures.end(), [&](const std::unique_ptr<Texture>& a, const std::unique_ptr<Texture>& b) {
					return attachmentMap[a->unit] < attachmentMap[b->unit];
				});
				attachments.resize(highestAttachment + 1, GL_NONE);
				attachments[attachmentOffset] = GL_COLOR_ATTACHMENT0 + attachmentOffset;
				std::sort(attachments.begin(), attachments.end(), [&](GLenum a, GLenum b) {
					return a < b;
				});

				GLenum attachment = attachments[attachmentOffset];
				Texture& texture = *textures[textures.size() - 1];
				
				gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);
				gl.framebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture.texture, 0);
				gl.drawBuffers(attachments.size(), &attachments[0]);
			}

			void addDepthBuffer() {
				gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);
				gl.genRenderbuffers(1, &depthBuffer);
				gl.bindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
				gl.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
				gl.framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
			}

			void resize(int _width, int _height) {
				width = _width;
				height = _height;

				for (auto& texture : textures)
					texture->resize(width, height);
				
				if (depthBuffer != -1) {
					gl.bindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
					gl.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
				}
			}

			void bind(int target) const {
				gl.bindFramebuffer(target, fbo);
			}

			void bind(bool viewport = false) const {
				bind(GL_FRAMEBUFFER);
				if (viewport) gl.viewport(0, 0, width, height);
			}
	};
}