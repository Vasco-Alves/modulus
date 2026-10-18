#include "modulus/gfx/graphics.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

#include <cstring>

namespace modulus::gfx {

	// The driver calls this when an OpenGL call goes wrong (debug builds only, see enable_debug_output)
	static void APIENTRY on_opengl_message(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum severity,
	                                       GLsizei /*length*/, const GLchar* message, const void* /*user_param*/) {
		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:   MOD_ERROR("OpenGL: {}", message); break; // Errors like GL_INVALID_ENUM
		case GL_DEBUG_SEVERITY_MEDIUM: MOD_WARN("OpenGL: {}", message);  break;
		case GL_DEBUG_SEVERITY_LOW:    MOD_TRACE("OpenGL: {}", message); break;
		default: break; // Notifications ("this buffer uses video memory", ...) are just noise
		}
	}

	static bool has_extension(const char* name) {
		GLint count = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &count);
		for (GLint i = 0; i < count; ++i) {
			if (std::strcmp((const char*)glGetStringi(GL_EXTENSIONS, i), name) == 0)
				return true;
		}
		return false;
	}

	// Asks the driver to report errors as they happen, with a description, instead of
	// silently ignoring the call. Needs OpenGL 4.3, or GL_KHR_debug, which most drivers
	// also offer on older versions.
	static void enable_debug_output(platform::IWindow& window) {
		if (!GLAD_GL_VERSION_4_3 && !has_extension("GL_KHR_debug"))
			return;

		// GLAD only loads OpenGL 4.3 functions on 4.3 contexts, so look this one up ourselves
		auto debug_message_callback = (PFNGLDEBUGMESSAGECALLBACKPROC)window.get_gl_loader()("glDebugMessageCallback");
		if (!debug_message_callback)
			return;

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Report during the call that went wrong, so a debugger shows which one
		debug_message_callback(on_opengl_message, nullptr);
	}

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

#ifndef NDEBUG
		enable_debug_output(window);
#endif

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