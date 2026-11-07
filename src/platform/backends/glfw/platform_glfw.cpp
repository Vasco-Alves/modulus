#include <modulus/platform/platform.hpp>
#include <modulus/core/log.hpp>
#include "glfw_internal.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <optional>

namespace modulus::platform {

	// =========================================================================
	// Helpers
	// =========================================================================

	// This function catches errors from inside GLFW (e.g. invalid monitor)
	// and pipes them into our own Logging system. (Not static: input_glfw.cpp uses it too.)
	void log_glfw_error(int error, const char* description) {
		MOD_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	static void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
		g_scroll_delta_y += (float)yoffset;
	}

	static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
		// Unknown keys are -1, so we skip those
		if (key < 0)
			return;

		// GLFW_PRESS is sent once when a key goes down. Holding it sends GLFW_REPEAT again and again
		// (after a short wait, as fast as the keyboard settings say): those count as repeated, not pressed.
		if (action == GLFW_PRESS)
			g_keys_pressed[key] = true;
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
			g_keys_repeated[key] = true;
	}

	// Adds one character to UTF-8 text. UTF-8 stores a character in 1 to 4 bytes: up to U+007F (plain
	// ASCII) in 1 byte as it is, bigger ones split into pieces of 6 bits, after a first byte that says
	// how many bytes there are. Every byte but the first looks like 10xxxxxx.
	static void append_utf8(std::string& text, char32_t c) {
		if (c < 0x80) {
			text += char(c);
		} else if (c < 0x800) {
			text += char(0xC0 | (c >> 6));
			text += char(0x80 | (c & 0x3F));
		} else if (c < 0x10000) {
			text += char(0xE0 | (c >> 12));
			text += char(0x80 | ((c >> 6) & 0x3F));
			text += char(0x80 | (c & 0x3F));
		} else {
			text += char(0xF0 | (c >> 18));
			text += char(0x80 | ((c >> 12) & 0x3F));
			text += char(0x80 | ((c >> 6) & 0x3F));
			text += char(0x80 | (c & 0x3F));
		}
	}

	// GLFW calls this for each character typed, as its Unicode number (its "code point"), after the
	// keyboard layout, Shift and accent keys. It already left out what isn't text: Enter, Backspace,
	// Escape and the other control characters, and shortcuts like Ctrl+V.
	static void char_callback(GLFWwindow* /*window*/, unsigned int codepoint) {
		// Numbers that aren't characters (half of a UTF-16 pair, or past the last character) are
		// skipped, so the text is always valid UTF-8
		if ((codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF)
			return;

		append_utf8(g_typed_text, char32_t(codepoint));
	}

	static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
		// Like g_keys_pressed in key_callback: only record the moment a button goes down (mouse buttons don't repeat)
		if (action == GLFW_PRESS && button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
			g_mouse_buttons_pressed[button] = true;
	}

	// GLFW calls this when the main window gets or loses the keyboard focus
	static void focus_callback(GLFWwindow* /*window*/, int focused) {
		g_main_window_focused = focused == GLFW_TRUE;
	}

	GLFWwindow* g_main_window_handle = nullptr;

	bool g_main_window_focused = false;

	float g_scroll_delta_y = 0.0f;

	bool g_keys_pressed[GLFW_KEY_LAST + 1] = {};

	bool g_keys_repeated[GLFW_KEY_LAST + 1] = {};

	bool g_mouse_buttons_pressed[GLFW_MOUSE_BUTTON_LAST + 1] = {};

	std::string g_typed_text;

	// Forgets the main window's input from the last frame: scroll, key presses and repeats, clicks
	// and typing. Done when a frame starts, and when the main window closes (so the next main window
	// starts with nothing pressed, even before its first update()).
	static void clear_frame_input() {
		g_scroll_delta_y = 0.0f;
		for (bool& pressed : g_keys_pressed)
			pressed = false;
		for (bool& repeated : g_keys_repeated)
			repeated = false;
		for (bool& pressed : g_mouse_buttons_pressed)
			pressed = false;
		g_typed_text.clear();
	}

	// How many windows are open right now.
	// GLFW starts when the first window opens and stops when the last one closes,
	// so users never have to start or stop the platform module themselves.
	static int s_window_count = 0;

	static bool start_glfw() {
		// Redirect GLFW errors to modulus logger
		glfwSetErrorCallback(log_glfw_error);

		if (!glfwInit()) {
			MOD_FATAL("Failed to initialize GLFW!");
			return false;
		}

		MOD_INFO("Platform initialized (Backend: GLFW)");

		// Say in the log which joysticks Input can't use as gamepads. (Stopping GLFW forgets its
		// joystick callback, so this runs every time GLFW starts.)
		watch_for_unknown_joysticks();
		return true;
	}

	static void stop_glfw() {
		glfwTerminate();
		MOD_INFO("Platform shutdown.");
	}

	// =========================================================================
	// Internal Class: The actual GLFW implementation
	// =========================================================================
	class GlfwWindow : public IWindow {
	public:
		GlfwWindow(const WindowConfig& config) {
			// The first window starts GLFW
			if (s_window_count == 0 && !start_glfw())
				return;

			// Hint to GLFW that we want a modern OpenGL context (3.3+ Core)
			// This is required for MacOS and recommended for modern Windows apps.
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

			// A hidden window still gets an OpenGL context, so it can also be used to draw offscreen
			glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);

			// Debug builds ask for a debug context: in one, the driver describes every OpenGL error
			// (the graphics module logs those descriptions)
#ifndef NDEBUG
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
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

				// If no other window is open, nothing needs GLFW anymore
				if (s_window_count == 0)
					stop_glfw();
				return;
			}

			s_window_count++;

			// The first window is the "main" window: the Input functions read from it
			if (g_main_window_handle == nullptr) {
				g_main_window_handle = m_handle;
				glfwSetScrollCallback(m_handle, scroll_callback);
				glfwSetKeyCallback(m_handle, key_callback);
				glfwSetCharCallback(m_handle, char_callback);
				glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
				glfwSetWindowFocusCallback(m_handle, focus_callback);

				// It may have the focus already: on Windows a visible window gets it while it's being
				// created, before the callback above existed
				g_main_window_focused = glfwGetWindowAttrib(m_handle, GLFW_FOCUSED) == GLFW_TRUE;
			}

			// Associate this class instance with the raw GLFW window,
			// so static callbacks (like framebuffer_size_callback) can find it again
			glfwSetWindowUserPointer(m_handle, this);
			glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);

			// GLFW creates the OpenGL context together with the window, so the window owns it.
			// Making it "current" means OpenGL calls from now on draw into this window.
			glfwMakeContextCurrent(m_handle);

			// Enable VSync if requested
			glfwSwapInterval(config.vsync ? 1 : 0);

			MOD_INFO("Window created: {0}x{1} ({2})", config.width, config.height, config.title);
		}

		~GlfwWindow() override {
			// Window creation failed, so there is nothing to clean up
			if (!m_handle)
				return;

			// Input reads nothing without a main window, and the next one starts afresh
			if (g_main_window_handle == m_handle) {
				g_main_window_handle = nullptr;
				g_main_window_focused = false;
				clear_frame_input();
				forget_gamepads();
			}

			glfwDestroyWindow(m_handle);
			m_handle = nullptr;

			// The last window to close stops GLFW
			s_window_count--;
			if (s_window_count == 0)
				stop_glfw();
		}

		bool update() override {
			if (!m_handle)
				return false;

			// The time since the last update() is how long the last frame took
			auto now = std::chrono::steady_clock::now();
			if (m_last_update)
				m_frame_time = std::chrono::duration<float>(now - *m_last_update).count();
			m_last_update = now;

			const bool is_main_window = m_handle == g_main_window_handle;

			// A new frame starts: forget the main window's scroll, key presses and repeats, clicks and typing from last frame
			if (is_main_window)
				clear_frame_input();

			// Process OS events (keyboard, typing, mouse, window resize, joysticks plugged in or out)
			glfwPollEvents();

			// Gamepad buttons and sticks send no events, so read them all now, once: every Input function
			// then sees the same buttons for the whole frame. (After glfwPollEvents(), which is when GLFW
			// hears about gamepads being plugged in or out.)
			if (is_main_window)
				update_gamepads(read_joysticks());

			return !glfwWindowShouldClose(m_handle);
		}

		void close() override {
			if (m_handle)
				glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
		}

		float get_frame_time() const override {
			return m_frame_time;
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

		std::pair<int, int> get_framebuffer_size() const override {
			int w, h;
			glfwGetFramebufferSize(m_handle, &w, &h);
			return { w, h };
		}

		void swap_buffers() override {
			if (m_handle)
				glfwSwapBuffers(m_handle);
		}

		GLLoader get_gl_loader() const override {
			// GLFW's function has a slightly different return type, so it needs a cast
			return (GLLoader)glfwGetProcAddress;
		}

		void set_framebuffer_resize_callback(FramebufferResizeCallback callback) override {
			m_framebuffer_resize_callback = callback;
		}

		void* get_native_handle() const override {
			return m_handle;
		}

	private:
		// GLFW calls this when the drawing area changes size.
		// It finds the GlfwWindow through the user pointer and passes the new size on.
		static void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
			auto* window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(handle));
			if (window && window->m_framebuffer_resize_callback)
				window->m_framebuffer_resize_callback(width, height);
		}

		GLFWwindow* m_handle = nullptr;
		FramebufferResizeCallback m_framebuffer_resize_callback = nullptr;

		std::optional<std::chrono::steady_clock::time_point> m_last_update; // Empty until the first update()
		float m_frame_time = 0.0f;
	};

	// =========================================================================
	// Factory Implementation
	// =========================================================================

	std::unique_ptr<IWindow> create_window(const WindowConfig& config) {
		auto window = std::make_unique<GlfwWindow>(config);

		// If creation failed, the error was already logged.
		// Return nothing, so the caller can check with: if (!window)
		if (!window->get_native_handle())
			return nullptr;

		return window;
	}

} // namespace modulus::platform