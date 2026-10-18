#pragma once

#include <modulus/gfx/sprite_sheet.hpp>

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <string_view>

namespace modulus::gfx {

	// The letters to draw text with (see Renderer2D::draw_text): a texture holding every
	// character ("glyph"), and where each one is in it.
	class Font {
	public:
		// One character of the font. Sizes are in the font's own pixels.
		struct Glyph {
			Sprite sprite;            // Where it is in the font's texture
			glm::vec2 offset;         // From the pen (where the character starts) to its top-left corner
			glm::vec2 size;           // How big it is
			float advance;            // How far it moves the pen to the right, for the next character
		};

		// A simple pixel font that is always there: letters, digits and punctuation (ASCII),
		// 5x8 pixels each with room around them, so a line is 10 pixels tall. It looks sharpest
		// at heights that are multiples of 10. Returns nullptr if its texture can't be made.
		static std::shared_ptr<Font> create_builtin();

		// The glyph for a character ('?' for characters the font doesn't have)
		const Glyph& get_glyph(char32_t character) const;

		// How tall one line is, in the font's pixels
		float get_line_height() const { return m_line_height; }

		// The size 'text' takes up when drawn 'height' tall: the width of its longest line,
		// and the height of all its lines together ('\n' starts a new line)
		glm::vec2 measure(std::string_view text, float height) const;

		// Reads one character of UTF-8 text, starting at byte 'index', and moves 'index' past it.
		// (In UTF-8, a character takes 1 to 4 bytes: 'a' takes 1, an accented letter like 'é' 2.)
		// Broken UTF-8 gives U+FFFD, the "replacement character", which fonts show as '?'.
		static char32_t next_character(std::string_view text, size_t& index);

	private:
		Font() = default;

		static constexpr char32_t first_character = ' ';  // The built-in font has these
		static constexpr char32_t last_character = '~';   // ASCII characters, in order

		std::array<Glyph, last_character - first_character + 1> m_glyphs = {};
		float m_line_height = 0.0f;
	};

} // namespace modulus::gfx
