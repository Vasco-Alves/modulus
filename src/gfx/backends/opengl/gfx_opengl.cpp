#include <modulus/gfx/graphics.hpp>
#include <modulus/core/log.hpp>

// Include GLAD first! (It includes standard GL headers)
#include <glad/glad.h>
#include <GLFW/glfw3.h> // Needed for glfwGetProcAddress and glfwSwapBuffers

namespace modulus::gfx {

	bool init() {
		// GLAD requires a context to be current BEFORE loading.
		// Since we create the Window (and context) before calling gfx::init(), 
		// this is safe *if* the user follows the correct order.

		// Load OpenGL pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			MOD_FATAL("Failed to initialize GLAD!");
			return false;
		}

		MOD_INFO("Graphics Initialized: OpenGL {0}", (const char*)glGetString(GL_VERSION));
		MOD_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		MOD_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));

		// Enable default states (optional)
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return true;
	}

	void shutdown() {
		// Nothing strictly needed for OpenGL/GLAD shutdown
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
		// We need the native GLFW handle to swap buffers
		GLFWwindow* handle = static_cast<GLFWwindow*>(window.get_native_handle());
		if (handle) {
			glfwSwapBuffers(handle);
		}
	}

} // namespace modulus::gfx