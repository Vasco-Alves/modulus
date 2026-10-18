#include "modulus/gfx/graphics.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

namespace modulus::gfx {

	bool init(platform::IWindow& window) {
		// OpenGL functions live inside the graphics driver, so their addresses have to be
		// looked up while the program runs. GLAD does that for us, using the loader
		// function that the window provides.
		if (!gladLoadGLLoader(window.get_gl_loader())) {
			MOD_FATAL("Failed to initialize GLAD!");
			return false;
		}

		MOD_INFO("Graphics Initialized: OpenGL {0}", (const char*)glGetString(GL_VERSION));
		MOD_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		MOD_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));

		// Enable default states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw into the whole window, and keep doing that when the window is resized.
		// (OpenGL doesn't do this by itself: without it, the old viewport size stays.)
		auto [width, height] = window.get_framebuffer_size();
		glViewport(0, 0, width, height);
		window.set_framebuffer_resize_callback([](int new_width, int new_height) {
			glViewport(0, 0, new_width, new_height);
		});

		return true;
	}

	void set_viewport(int x, int y, int width, int height) {
		glViewport(x, y, width, height);
	}

	void clear(const Color& color) {
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void draw_indexed(const std::shared_ptr<VertexArray>& vertexArray) {
		vertexArray->bind();
		unsigned int count = vertexArray->get_index_buffer()->get_count();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void present(platform::IWindow& window) {
		// In OpenGL, showing the frame is the window's job
		window.swap_buffers();
	}

} // namespace modulus::gfx