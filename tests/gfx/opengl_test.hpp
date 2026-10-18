#pragma once

// Shared by the tests that need a real OpenGL 3.3 context. Each test opens a hidden window;
// where that isn't possible (no GPU or no display, like on many CI servers) the test skips itself.

#include <glad/glad.h> // Before GLFW, so GLFW doesn't include the system's OpenGL header
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <modulus/gfx/graphics.hpp>
#include <modulus/platform/platform.hpp>

#include <array>
#include <cstdlib>
#include <memory>

class OpenGLTest : public ::testing::Test {
protected:
	static constexpr int window_width = 320;
	static constexpr int window_height = 240;

	void SetUp() override {
		modulus::platform::WindowConfig config;
		config.title = "modulus_tests";
		config.width = window_width;
		config.height = window_height;
		config.visible = false;

		window = modulus::platform::create_window(config);
		if (!window || !modulus::gfx::init(*window))
			GTEST_SKIP() << "No OpenGL 3.3 context available";
	}

	std::unique_ptr<modulus::platform::IWindow> window;

	// Data for the buffers. Most tests only check how buffers are set up, not what is in them.
	float vertices[64] = {};
	unsigned int indices[6] = { 0, 1, 2, 2, 3, 0 };
};

inline GLint get_integer(GLenum name) {
	GLint value = 0;
	glGetIntegerv(name, &value);
	return value;
}

// Reads a property of vertex attribute 'index' of the bound vertex array
inline GLint get_attribute(GLuint index, GLenum name) {
	GLint value = 0;
	glGetVertexAttribiv(index, name, &value);
	return value;
}

// An image to draw into instead of the window, so a test can read the pixels back
// (a hidden window may have no pixels to read). It also sets the viewport to its size.
class OffscreenTarget {
public:
	OffscreenTarget(int width, int height) : m_height(height) {
		glGenFramebuffers(1, &m_framebuffer);
		glGenRenderbuffers(1, &m_color_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_color_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_color_buffer);
		glViewport(0, 0, width, height);
	}

	~OffscreenTarget() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteRenderbuffers(1, &m_color_buffer);
		glDeleteFramebuffers(1, &m_framebuffer);
	}

	OffscreenTarget(const OffscreenTarget&) = delete;
	OffscreenTarget& operator=(const OffscreenTarget&) = delete;

	bool is_complete() const {
		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	// The color of one pixel (red, green, blue, alpha from 0 to 255).
	// x and y count from the top-left corner, like the 2D renderer. (OpenGL counts from the bottom-left.)
	std::array<int, 4> pixel(int x, int y) const {
		unsigned char rgba[4] = {};
		glReadPixels(x, m_height - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
		return { rgba[0], rgba[1], rgba[2], rgba[3] };
	}

private:
	GLuint m_framebuffer = 0;
	GLuint m_color_buffer = 0;
	int m_height;
};

// Checks a pixel's color, allowing a small difference (texture filtering blends neighboring texels a little)
inline ::testing::AssertionResult has_color(const std::array<int, 4>& pixel, int r, int g, int b, int tolerance = 40) {
	if (std::abs(pixel[0] - r) <= tolerance && std::abs(pixel[1] - g) <= tolerance && std::abs(pixel[2] - b) <= tolerance)
		return ::testing::AssertionSuccess();

	return ::testing::AssertionFailure() << "the pixel is (" << pixel[0] << ", " << pixel[1] << ", " << pixel[2]
		<< "), expected about (" << r << ", " << g << ", " << b << ")";
}
