#include <modulus/core/log.hpp>
#include <modulus/platform/platform.hpp>

int main() {
	// 1. Initialize Logging
	MOD_INFO("Starting Hello Window Example...");

	// 2. Initialize Platform (GLFW)
	if (!modulus::platform::init()) {
		return -1;
	}

	// 3. Create a Window
	modulus::platform::WindowConfig config;
	config.title = "Hello Modulus!";
	config.width = 800;
	config.height = 600;

	auto window = modulus::platform::create_window(config);
	if (!window) {
		modulus::platform::shutdown();
		return -1;
	}

	// 4. Main Loop
	while (window->update()) {
		// In the future:
		// gfx::clear();
		// game::update();
		// gfx::present();
	}

	// 5. Cleanup
	window.reset(); // Destroy window before shutting down GLFW
	modulus::platform::shutdown();

	return 0;
}