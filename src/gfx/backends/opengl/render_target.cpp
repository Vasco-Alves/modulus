#include "modulus/gfx/render_target.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

#include <cstdint>
#include <vector>

namespace modulus::gfx {

	std::unique_ptr<RenderTarget> RenderTarget::create(int width, int height) {
		if (width <= 0 || height <= 0) {
			MOD_ERROR("Can't make a render target of {}x{} pixels", width, height);
			return nullptr;
		}

		// The constructor is private (only create() makes render targets), so std::make_unique can't call it
		std::unique_ptr<RenderTarget> target(new RenderTarget());
		target->m_width = width;
		target->m_height = height;

		// The image: an empty (transparent) texture of that size
		std::vector<uint8_t> empty(size_t(width) * height * 4, 0);
		target->m_texture = Texture2D::create(width, height, empty.data());
		if (!target->m_texture)
			return nullptr; // The error was already logged
		target->m_texture->set_filter(TextureFilter::Nearest);

		// A framebuffer is what OpenGL draws into. Make one that draws into the texture...
		GLint previous_framebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);
		glGenFramebuffers(1, &target->m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, target->m_framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->m_texture->get_renderer_id(), 0);

		// ...and has a depth buffer, like a window does
		glGenRenderbuffers(1, &target->m_depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, target->m_depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target->m_depth_buffer);

		bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
		if (!complete) {
			MOD_ERROR("Failed to make a render target of {}x{} pixels", width, height);
			return nullptr; // The destructor frees what was made
		}

		target->clear({ 0.0f, 0.0f, 0.0f, 0.0f });
		return target;
	}

	RenderTarget::~RenderTarget() {
		glDeleteRenderbuffers(1, &m_depth_buffer);
		glDeleteFramebuffers(1, &m_framebuffer);
	}

	void RenderTarget::clear(const Color& color) {
		// Clear this target without changing what's being drawn into
		GLint previous_framebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
	}

	void RenderTarget::bind() {
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_previous_framebuffer);
		glGetIntegerv(GL_VIEWPORT, m_previous_viewport);

		// Draw into this target, all of it
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		glViewport(0, 0, m_width, m_height);
	}

	void RenderTarget::unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, m_previous_framebuffer);
		glViewport(m_previous_viewport[0], m_previous_viewport[1], m_previous_viewport[2], m_previous_viewport[3]);
	}

} // namespace modulus::gfx
