// Hello 2D: the 2D renderer and camera.
//   Move the mouse: the white square follows it.
//   Scroll: zoom toward the mouse.
//   Hold the right mouse button: drag the view around.
//   Escape: quit.

#include <modulus/core/timer.hpp>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/renderer_2d.hpp>
#include <modulus/gfx/texture.hpp>

#include <cstdint>
#include <format>
#include <vector>

using namespace modulus;

int main() {
	platform::WindowConfig config;
	config.title = "Modulus - Hello 2D";

	auto window = platform::create_window(config);
	if (!window || !gfx::init(*window))
		return -1;

	auto renderer = gfx::Renderer2D::create(*window);
	if (!renderer)
		return -1;

	// A checkerboard texture, made in code so this example needs no image file
	const int size = 8;
	std::vector<uint8_t> pixels;
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			uint8_t shade = (x + y) % 2 == 0 ? 255 : 90;
			pixels.insert(pixels.end(), { shade, shade, shade, 255 }); // Red, green, blue, alpha
		}
	}
	auto checkerboard = gfx::Texture2D::create(size, size, pixels.data());
	if (!checkerboard)
		return -1;
	checkerboard->set_filter(gfx::TextureFilter::Nearest); // Crisp squares instead of a blur when drawn big

	gfx::Camera2D camera;
	core::Timer timer;
	glm::vec2 last_mouse = { 0.0f, 0.0f };

	while (window->update()) {
		if (platform::Input::is_key_pressed(platform::Key::Escape))
			window->close();

		auto [mouse_x, mouse_y] = platform::Input::get_mouse_position();
		glm::vec2 mouse = { mouse_x, mouse_y };

		// Scroll to zoom toward the mouse
		float scroll = platform::Input::get_mouse_scroll();
		if (scroll != 0.0f)
			camera.zoom_at(mouse, camera.zoom * (scroll > 0.0f ? 1.1f : 1.0f / 1.1f));

		// Drag with the right mouse button to move the view (the world moves with the mouse)
		if (platform::Input::is_mouse_down(platform::MouseButton::Right))
			camera.position -= (mouse - last_mouse) / camera.zoom;
		last_mouse = mouse;

		gfx::clear({ 0.1f, 0.1f, 0.12f, 1.0f });
		renderer->begin(camera);

		// A grid of colored squares
		for (int y = 0; y < 12; ++y) {
			for (int x = 0; x < 20; ++x) {
				gfx::Color color = { x / 20.0f, y / 12.0f, 0.6f, 1.0f };
				renderer->draw_quad({ 40.0f + x * 50.0f, 40.0f + y * 50.0f }, { 44.0f, 44.0f }, color);
			}
		}

		// A textured square on top, centered on (640, 360) and turning around its center, 45 degrees per second
		renderer->draw_quad({ 640.0f, 360.0f }, { 200.0f, 200.0f }, *checkerboard,
		                    { .origin = { 0.5f, 0.5f }, .rotation = timer.elapsed() * 45.0f });

		// A white square under the mouse. The mouse position is a point on the window,
		// so turn it into a point in the world first (they differ once you zoom or drag).
		glm::vec2 cursor = camera.screen_to_world(mouse);
		renderer->draw_quad(cursor - glm::vec2(8.0f, 8.0f), { 16.0f, 16.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

		renderer->end();

		// Text on top, drawn without the camera, so it stays in place when you zoom or drag the view
		renderer->begin();
		renderer->draw_text({ 40.0f, 670.0f },
		                    std::format("Scroll: zoom ({:.1f}x)    Right mouse button: move the view", camera.zoom),
		                    20.0f, { 0.8f, 0.8f, 0.85f, 1.0f });
		renderer->end();
		gfx::present(*window);
	}

	return 0;
}
