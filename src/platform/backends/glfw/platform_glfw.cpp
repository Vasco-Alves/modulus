#include <modulus/platform/platform.hpp>
#include <modulus/core/log.hpp>
#include "glfw_internal.hpp"

#include <GLFW/glfw3.h>

namespace modulus::platform {

	// =========================================================================
	// Helpers
	// =========================================================================

	// This function catches errors from inside GLFW (e.g. invalid monitor)
	// and pipes them into our own Logging system.
	static void glfw_error_callback(int error, const char* description) {
		MOD_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	GLFWwindow* g_mainWindowHandle = nullptr;

	// =========================================================================
	// Internal Class: The actual GLFW implementation
	// =========================================================================
	class GlfwWindow : public IWindow {
	public:
		GlfwWindow(const WindowConfig& config) {
			// Hint to GLFW that we want a modern OpenGL context (3.3+ Core)
			// This is required for MacOS and recommended for modern Windows apps.
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

			// Create the window
			m_handle = glfwCreateWindow(
				config.width,
				config.height,
				config.title.c_str(),
				nullptr, // Monitor (nullptr = windowed mode)
				nullptr  // Share
			);

			if (!m_handle) {
				MOD_FATAL("Failed to create GLFW window!");
				// In a real engine, we might throw or return nullptr, 
				// but for MVP, logging fatal is enough context.
				return;
			}

			if (g_mainWindowHandle == nullptr) {
				g_mainWindowHandle = m_handle;
			}

			// Associate this class instance with the raw GLFW window
			// (Useful if we need to access 'this' inside static callbacks later)
			glfwSetWindowUserPointer(m_handle, this);

			// TODO: Move this to Gfx module later.
			// For now, the window owns the context so we can swap buffers.
			glfwMakeContextCurrent(m_handle);

			// Enable VSync if requested
			glfwSwapInterval(config.vsync ? 1 : 0);

			MOD_INFO("Window created: {0}x{1} ({2})", config.width, config.height, config.title);
		}

		~GlfwWindow() override {
			if (g_mainWindowHandle == m_handle) {
				g_mainWindowHandle = nullptr;
			}

			if (m_handle) {
				glfwDestroyWindow(m_handle);
				m_handle = nullptr;
			}
		}

		bool update() override {
			if (!m_handle)
				return false;

			// Process OS events (keyboard, mouse, window resize)
			glfwPollEvents();

			return !glfwWindowShouldClose(m_handle);
		}

		void close() override {
			if (m_handle)
				glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
		}

		void set_title(const std::string& title) override {
			if (m_handle)
				glfwSetWindowTitle(m_handle, title.c_str());
		}

		int width() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return w;
		}

		int height() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return h;
		}

		std::pair<int, int> get_size() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return { w, h };
		}

		void* get_native_handle() const override {
			return m_handle;
		}

	private:
		GLFWwindow* m_handle = nullptr;
	};

	// =========================================================================
	// Factory Implementation
	// =========================================================================

	bool init() {
		// Redirect GLFW errors to modulus logger
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			MOD_FATAL("Failed to initialize GLFW!");
			return false;
		}

		MOD_INFO("Platform initialized (Backend: GLFW)");
		return true;
	}

	void shutdown() {
		glfwTerminate();
		MOD_INFO("Platform shutdown.");
	}

	std::unique_ptr<IWindow> create_window(const WindowConfig& config) {
		return std::make_unique<GlfwWindow>(config);
	}

} // namespace modulus::platform