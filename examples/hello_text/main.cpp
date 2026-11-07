// Hello Text: drawing text with a TrueType font and with the built-in pixel font, and typing some.
//   Type your name and press Enter. Backspace deletes (hold it to keep deleting),
//   Ctrl+V pastes and Ctrl+C copies the name.
//   Escape: quit.

#include <modulus/core/file.hpp>
#include <modulus/core/timer.hpp>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <modulus/gfx/font.hpp>
#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/renderer_2d.hpp>

#include <cmath>
#include <cstdint>
#include <string>

using namespace modulus;
using platform::Input;
using platform::Key;

// Removes the last character of UTF-8 text. A character can take several bytes ('é' takes 2), so
// text.pop_back() alone could cut one in half, and half a character is drawn as '?'. Every byte of
// a character but the first looks like 10xxxxxx in binary: remove those, then the first one.
static void remove_last_character(std::string& text) {
	while (!text.empty() && (uint8_t(text.back()) & 0xC0) == 0x80)
		text.pop_back();
	if (!text.empty())
		text.pop_back();
}

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

	// The name field
	std::string name;     // What's been typed so far
	std::string greeting; // Set when Enter is pressed
	const glm::vec2 box_position = { 70.0f, 620.0f };
	const glm::vec2 box_size = { 560.0f, 50.0f };

	while (window->update()) {
		if (Input::is_key_pressed(Key::Escape))
			window->close();

		// What was typed this frame: usually nothing, or one character. It follows the keyboard layout,
		// Shift and accent keys, and never holds Enter or Backspace (those are keys, not text).
		std::string typed = Input::get_typed_text();

		// Ctrl+V pastes the clipboard's first line (a name has just one), and Ctrl+C copies the name.
		// The clipboard is only read when pasting: on Linux, that means asking the program that has it.
		bool control = Input::is_key_down(Key::LeftControl) || Input::is_key_down(Key::RightControl);
		if (control && Input::is_key_pressed(Key::V)) {
			std::string pasted = Input::get_clipboard_text();
			typed += pasted.substr(0, pasted.find_first_of("\r\n"));
		}
		if (control && Input::is_key_pressed(Key::C))
			Input::set_clipboard_text(name);

		// Added to the name, as long as it still fits in its box
		if (font->measure(name + typed, 32.0f).x <= box_size.x - 20.0f)
			name += typed;

		// Backspace deletes the last character. is_key_repeated() is true again each time a held key
		// repeats, so holding Backspace keeps deleting, like in any text field.
		if (Input::is_key_repeated(Key::Backspace))
			remove_last_character(name);

		// Enter, like a form's OK button
		if (Input::is_key_pressed(Key::Enter) && !name.empty())
			greeting = "Hello, " + name + "!";

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

		// The name field: what to do (or the greeting), a box with the name in it, and a blinking line
		// after the name, where the next letter goes. (The font only has Western European letters, so
		// others, like Japanese, show as '?'.)
		std::string label = greeting.empty() ? "Type your name and press Enter:" : greeting;
		renderer->draw_text({ 80.0f, 572.0f }, label, *font, 32.0f, greeting.empty() ? gray : gold);
		renderer->draw_quad(box_position, box_size, { 0.2f, 0.25f, 0.4f, 1.0f });

		glm::vec2 text_position = box_position + glm::vec2(10.0f, 7.0f);
		renderer->draw_text(text_position, name, *font, 32.0f, white);
		if (std::fmod(timer.elapsed(), 1.0f) < 0.5f) { // Shown for half of every second
			float cursor_x = text_position.x + font->measure(name, 32.0f).x + 2.0f;
			renderer->draw_quad({ cursor_x, text_position.y }, { 2.0f, 34.0f }, white);
		}

		renderer->end();
		gfx::present(*window);
	}

	return 0;
}
