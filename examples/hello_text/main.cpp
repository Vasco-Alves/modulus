// Hello Text: drawing text with a TrueType font and with the built-in pixel font.
//   Escape: quit.

#include <modulus/core/file.hpp>
#include <modulus/core/timer.hpp>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <modulus/gfx/font.hpp>
#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/renderer_2d.hpp>

#include <string>

using namespace modulus;

int main() {
	platform::WindowConfig config;
	config.title = "Modulus - Hello Text";

	auto window = platform::create_window(config);
	if (!window || !gfx::init(*window))
		return -1;

	auto renderer = gfx::Renderer2D::create(*window);
	if (!renderer)
		return -1;

	// A TrueType font, loaded at the two heights this example draws it at
	// (text is sharpest at the height its font was loaded at)
	std::string font_path = core::File::app_path("assets/fonts/Lato-Regular.ttf");
	auto font = gfx::Font::load(font_path, 32.0f);
	auto title_font = gfx::Font::load(font_path, 72.0f);
	if (!font || !title_font)
		return -1;

	const gfx::Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
	const gfx::Color gray = { 0.6f, 0.6f, 0.65f, 1.0f };
	const gfx::Color gold = { 1.0f, 0.8f, 0.3f, 1.0f };
	const gfx::Color sky = { 0.45f, 0.75f, 1.0f, 1.0f };
	const gfx::DrawOptions centered = { .origin = { 0.5f, 0.5f } };

	core::Timer timer;

	while (window->update()) {
		if (platform::Input::is_key_pressed(platform::Key::Escape))
			window->close();

		gfx::clear({ 0.1f, 0.1f, 0.12f, 1.0f });
		renderer->begin();

		// A title, centered at the top
		renderer->draw_text({ 640.0f, 80.0f }, "Hello, text!", *title_font, 72.0f, white, centered);

		// The two kinds of font
		renderer->draw_text({ 80.0f, 170.0f }, "A TrueType font (Lato): smooth, and each letter as wide as it needs", *font, 32.0f, white);
		renderer->draw_text({ 80.0f, 220.0f }, "The built-in pixel font: no file needed", 30.0f, gray);

		// TrueType fonts have the accented letters of Western European languages
		renderer->draw_text({ 80.0f, 290.0f }, "Olá! Ação, coração e pão. Déjà vu, señor, Größe.", *font, 32.0f, gold);

		// measure() says how much room a text takes, for example to fit a box behind it
		const char* boxed = "This box is exactly as big as its text";
		glm::vec2 size = font->measure(boxed, 32.0f);
		renderer->draw_quad({ 70.0f, 355.0f }, size + glm::vec2(20.0f, 10.0f), { 0.2f, 0.25f, 0.4f, 1.0f });
		renderer->draw_text({ 80.0f, 360.0f }, boxed, *font, 32.0f, white);

		// Several lines
		renderer->draw_text({ 80.0f, 440.0f }, "Text can have\nseveral lines,\none below the other.", *font, 32.0f, sky);

		// Turning around its center
		renderer->draw_text({ 900.0f, 520.0f }, "Round and round", *font, 32.0f, sky,
		                    { .origin = { 0.5f, 0.5f }, .rotation = timer.elapsed() * 45.0f });

		renderer->end();
		gfx::present(*window);
	}

	return 0;
}
