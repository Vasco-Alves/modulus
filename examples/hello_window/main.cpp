/*
#include <modulus/core/log.hpp>
#include <modulus/platform/platform.hpp>

int main() {
	MOD_INFO("Starting Hello Window Example...");

	// 1. Initialize Platform (GLFW)
	if (!modulus::platform::init()) {
		return -1;
	}

	// 2. Create a Window
	modulus::platform::WindowConfig config;
	config.title = "Hello Modulus!";
	config.width = 800;
	config.height = 600;

	auto window = modulus::platform::create_window(config);
	if (!window) {
		modulus::platform::shutdown();
		return -1;
	}

	// 3. Main Loop
	while (window->update()) {
		// In the future:
		// gfx::clear();
		// game::update();
		// gfx::present();
	}

	// 4. Cleanup
	window.reset(); // Destroy window before shutting down GLFW
	modulus::platform::shutdown();

	return 0;
}
*/

#include <modulus/core/context.hpp>
#include <modulus/platform/platform.hpp>
#include <modulus/platform/input.hpp>
#include <modulus/gfx/graphics.hpp>

int main() {
	modulus::Context ctx;
	if (!ctx.is_valid()) return -1;

	modulus::platform::WindowConfig config;
	config.title = "Modulus Graphics";
	auto window = modulus::platform::create_window(config);
	if (!window) return -1;

	if (!modulus::gfx::init()) return -1;

	while (window->update()) {
		// Update
		{
			// Input
			if (modulus::platform::Input::is_key_down(modulus::platform::Key::Escape)) {
				window->close();
			}
		}

		// Draw 
		{
			modulus::gfx::clear({ 0.2f, 0.4f, 0.8f, 1.0f });
			modulus::gfx::present(*window);
		}
	}

	return 0;
}