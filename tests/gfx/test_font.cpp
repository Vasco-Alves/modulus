// Fonts (the built-in one and TrueType ones) and reading UTF-8 text.
// Most of these need a real texture, so an OpenGL context (see opengl_test.hpp).

#include "opengl_test.hpp"

#include <modulus/gfx/font.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
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

TEST(Utf8, ReadsCharactersOfEveryLength) {
	// 'a' (1 byte), an accented e (2 bytes), the euro sign (3), and an emoji (4)
	std::string_view text = "a\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80";
	size_t index = 0;

	EXPECT_EQ(gfx::Font::next_character(text, index), char32_t('a'));
	EXPECT_EQ(gfx::Font::next_character(text, index), char32_t(0xE9));
	EXPECT_EQ(gfx::Font::next_character(text, index), char32_t(0x20AC));
	EXPECT_EQ(gfx::Font::next_character(text, index), char32_t(0x1F600));
	EXPECT_EQ(index, text.size());
}

TEST(Utf8, BrokenTextGivesReplacementCharacters) {
	std::string_view stray_continuation = "\x80" "a"; // A byte that only continues a character
	size_t index = 0;
	EXPECT_EQ(gfx::Font::next_character(stray_continuation, index), char32_t(0xFFFD));
	EXPECT_EQ(gfx::Font::next_character(stray_continuation, index), char32_t('a')); // Then it carries on

	std::string_view cut_short = "\xC3"; // The start of a 2-byte character, and then nothing
	index = 0;
	EXPECT_EQ(gfx::Font::next_character(cut_short, index), char32_t(0xFFFD));
	EXPECT_EQ(index, cut_short.size());
}

TEST_F(FontTest, LoadsATrueTypeFont) {
	auto lato = gfx::Font::load(MODULUS_TEST_FONT, 32.0f);
	ASSERT_NE(lato, nullptr);
	EXPECT_FLOAT_EQ(lato->get_line_height(), 32.0f);

	// Its characters have different widths: narrow letters take less room than wide ones
	EXPECT_LT(lato->measure("iiii", 32.0f).x, lato->measure("WWWW", 32.0f).x);

	// A capital letter is a bit more than half a line tall, and stands below the top of the line
	const gfx::Font::Glyph& a = lato->get_glyph('A');
	EXPECT_GT(a.size.y, 16.0f);
	EXPECT_LT(a.size.y, 32.0f);
	EXPECT_GT(a.offset.y, 0.0f);
}

TEST_F(FontTest, TrueTypeFontsHaveAccentedLetters) {
	auto lato = gfx::Font::load(MODULUS_TEST_FONT, 32.0f);
	ASSERT_NE(lato, nullptr);

	auto same_glyph = [](const gfx::Font::Glyph& first, const gfx::Font::Glyph& second) {
		return first.sprite.region.x == second.sprite.region.x && first.sprite.region.y == second.sprite.region.y;
	};
	const gfx::Font::Glyph& question_mark = lato->get_glyph('?');

	EXPECT_FALSE(same_glyph(lato->get_glyph(0xE7), question_mark)); // c with a cedilla has its own glyph
	EXPECT_FALSE(same_glyph(lato->get_glyph(0xE3), question_mark)); // So does a with a tilde
	EXPECT_TRUE(same_glyph(lato->get_glyph(0x4E2D), question_mark)); // Chinese is beyond Latin-1
	EXPECT_TRUE(same_glyph(lato->get_glyph(0x85), question_mark));   // An invisible control code
}

TEST_F(FontTest, TrueTypeFontsKernPairsLikeAV) {
	auto lato = gfx::Font::load(MODULUS_TEST_FONT, 32.0f);
	ASSERT_NE(lato, nullptr);

	// "AV" and "To" go closer together than their advances alone would put them
	float kerning = lato->get_kerning('A', 'V');
	EXPECT_LT(kerning, 0.0f);
	EXPECT_LT(lato->get_kerning('T', 'o'), 0.0f);

	// measure() counts it, scaled with the text...
	float advances = lato->get_glyph('A').advance + lato->get_glyph('V').advance;
	EXPECT_FLOAT_EQ(lato->measure("AV", 32.0f).x, advances + kerning);
	EXPECT_FLOAT_EQ(lato->measure("AV", 64.0f).x, 2.0f * (advances + kerning));

	// ...but only between characters next to each other on the same line
	EXPECT_FLOAT_EQ(lato->measure("A\nVVVV", 32.0f).x, lato->measure("VVVV", 32.0f).x);

	// Characters the font doesn't have (drawn as '?') aren't kerned, and the built-in font has no kerning
	EXPECT_FLOAT_EQ(lato->get_kerning('A', 0x4E2D), 0.0f);
	EXPECT_FLOAT_EQ(font->get_kerning('A', 'V'), 0.0f);
}

TEST_F(FontTest, LoadingAMissingOrBrokenFontFails) {
	// A file that isn't a font
	std::filesystem::path not_a_font = std::filesystem::temp_directory_path() / "modulus_test_not_a_font.ttf";
	{
		std::ofstream out(not_a_font, std::ios::binary);
		out << "This is just some text, not a font. It is long enough to have a table of contents.";
	}

	testing::internal::CaptureStderr();
	EXPECT_EQ(gfx::Font::load("this/font/does/not/exist.ttf", 32.0f), nullptr);
	EXPECT_EQ(gfx::Font::load(not_a_font.string(), 32.0f), nullptr);
	EXPECT_EQ(gfx::Font::load(MODULUS_TEST_FONT, 0.0f), nullptr);
	std::string errors = testing::internal::GetCapturedStderr();
	std::filesystem::remove(not_a_font);

	EXPECT_NE(errors.find("Failed to open font"), std::string::npos) << errors;
	EXPECT_NE(errors.find("Not a TrueType font"), std::string::npos) << errors;
	EXPECT_NE(errors.find("pixel height must be above 0"), std::string::npos) << errors;
}
