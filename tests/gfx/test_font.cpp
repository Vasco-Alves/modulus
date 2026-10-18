// The built-in font. It needs a real texture (see opengl_test.hpp).

#include "opengl_test.hpp"

#include <modulus/gfx/font.hpp>

#include <cstdint>
#include <memory>
#include <vector>

using namespace modulus;

namespace {

	class FontTest : public OpenGLTest {
	protected:
		void SetUp() override {
			OpenGLTest::SetUp();
			if (IsSkipped())
				return;

			font = gfx::Font::create_builtin();
			ASSERT_NE(font, nullptr);
		}

		std::shared_ptr<gfx::Font> font;
	};

	void expect_size(glm::vec2 actual, float width, float height) {
		EXPECT_FLOAT_EQ(actual.x, width);
		EXPECT_FLOAT_EQ(actual.y, height);
	}

} // namespace

TEST_F(FontTest, MeasuresText) {
	// Every character is 6 pixels wide (5 + a gap) and a line is 10 pixels tall
	expect_size(font->measure("Hi", 10.0f), 12.0f, 10.0f);
	expect_size(font->measure("Hi", 20.0f), 24.0f, 20.0f);           // Twice as big
	expect_size(font->measure("Hi\nHello", 10.0f), 30.0f, 20.0f);   // Widest line, two lines
	expect_size(font->measure("", 10.0f), 0.0f, 10.0f);
}

TEST_F(FontTest, CharactersItDoesNotHaveShowAsOneQuestionMarkEach) {
	// An accented e is 2 bytes in UTF-8, and an arrow is 3, but each is one character
	expect_size(font->measure("caf\xC3\xA9 \xE2\x86\x92", 10.0f), 36.0f, 10.0f);

	const gfx::Rect& question_mark = font->get_glyph('?').sprite.region;
	const gfx::Rect& unknown = font->get_glyph(char32_t(0xE9)).sprite.region; // An accented e
	EXPECT_FLOAT_EQ(unknown.x, question_mark.x);
	EXPECT_FLOAT_EQ(unknown.y, question_mark.y);
}

TEST_F(FontTest, GlyphsAreInASCIIOrder) {
	// 'A' is character 33 after ' ': column 1, row 2 of a grid with 16 cells of 6x10 per row
	const gfx::Rect& region = font->get_glyph('A').sprite.region;
	EXPECT_FLOAT_EQ(region.x, 6.0f);
	EXPECT_FLOAT_EQ(region.y, 20.0f);
	EXPECT_FLOAT_EQ(region.width, 6.0f);
	EXPECT_FLOAT_EQ(region.height, 10.0f);
}

TEST_F(FontTest, EveryCharacterHasPixelsInsideItsCell) {
	const gfx::Texture2D& texture = *font->get_glyph('A').sprite.texture;
	int width = texture.get_width();
	int height = texture.get_height();

	std::vector<uint8_t> pixels(size_t(width) * height * 4);
	texture.bind();
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	// Is the pixel at (x, y), counted from the top-left, opaque? (OpenGL stores the bottom row first.)
	auto opaque = [&](int x, int y) { return pixels[(size_t(height - 1 - y) * width + x) * 4 + 3] != 0; };

	for (char32_t character = ' '; character <= '~'; ++character) {
		const gfx::Rect& cell = font->get_glyph(character).sprite.region;
		int inside = 0, outside = 0;
		for (int y = 0; y < 10; ++y) {
			for (int x = 0; x < 6; ++x) {
				bool in_glyph_area = x < 5 && y < 8;
				if (opaque(int(cell.x) + x, int(cell.y) + y))
					(in_glyph_area ? inside : outside)++;
			}
		}

		EXPECT_EQ(outside, 0) << "character " << int(character) << " spills into the gap around it";
		if (character == ' ')
			EXPECT_EQ(inside, 0);
		else
			EXPECT_GT(inside, 0) << "character " << int(character) << " has no pixels";
	}
}
