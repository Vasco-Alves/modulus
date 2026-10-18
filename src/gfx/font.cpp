#include "modulus/gfx/font.hpp"
#include "modulus/gfx/texture.hpp"
#include "modulus/core/log.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

// This define tells the compiler: "Build the library code here!"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace modulus::gfx {

	// The built-in font's characters, from ' ' to '~' in ASCII order. Each one is 5 pixels wide and
	// 8 tall, one string per row from the top, and '#' is a pixel. Capitals and digits use the top
	// 7 rows; the 8th row is for the tails of g, j, p, q, y and the like.
	static const char* const builtin_glyphs[95][8] = {
		{ ".....", ".....", ".....", ".....", ".....", ".....", ".....", "....." }, // ' '
		{ "..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#..", "....." }, // '!'
		{ ".#.#.", ".#.#.", ".#.#.", ".....", ".....", ".....", ".....", "....." }, // '"'
		{ ".#.#.", ".#.#.", "#####", ".#.#.", "#####", ".#.#.", ".#.#.", "....." }, // '#'
		{ "..#..", ".####", "#.#..", ".###.", "..#.#", "####.", "..#..", "....." }, // '$'
		{ "##...", "##..#", "...#.", "..#..", ".#...", "#..##", "...##", "....." }, // '%'
		{ ".##..", "#..#.", "#.#..", ".#...", "#.#.#", "#..#.", ".##.#", "....." }, // '&'
		{ "..#..", "..#..", ".#...", ".....", ".....", ".....", ".....", "....." }, // '''
		{ "...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#.", "....." }, // '('
		{ ".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#...", "....." }, // ')'
		{ ".....", "..#..", "#.#.#", ".###.", "#.#.#", "..#..", ".....", "....." }, // '*'
		{ ".....", "..#..", "..#..", "#####", "..#..", "..#..", ".....", "....." }, // '+'
		{ ".....", ".....", ".....", ".....", ".....", ".##..", "..#..", ".#..." }, // ','
		{ ".....", ".....", ".....", "#####", ".....", ".....", ".....", "....." }, // '-'
		{ ".....", ".....", ".....", ".....", ".....", ".##..", ".##..", "....." }, // '.'
		{ ".....", "....#", "...#.", "..#..", ".#...", "#....", ".....", "....." }, // '/'
		{ ".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###.", "....." }, // '0'
		{ "..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###.", "....." }, // '1'
		{ ".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####", "....." }, // '2'
		{ "#####", "...#.", "..#..", "...#.", "....#", "#...#", ".###.", "....." }, // '3'
		{ "...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#.", "....." }, // '4'
		{ "#####", "#....", "####.", "....#", "....#", "#...#", ".###.", "....." }, // '5'
		{ "..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###.", "....." }, // '6'
		{ "#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#...", "....." }, // '7'
		{ ".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###.", "....." }, // '8'
		{ ".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##..", "....." }, // '9'
		{ ".....", ".##..", ".##..", ".....", ".##..", ".##..", ".....", "....." }, // ':'
		{ ".....", ".##..", ".##..", ".....", ".##..", "..#..", ".#...", "....." }, // ';'
		{ "...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#.", "....." }, // '<'
		{ ".....", ".....", "#####", ".....", "#####", ".....", ".....", "....." }, // '='
		{ ".#...", "..#..", "...#.", "....#", "...#.", "..#..", ".#...", "....." }, // '>'
		{ ".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#..", "....." }, // '?'
		{ ".###.", "#...#", "....#", ".##.#", "#.#.#", "#.#.#", ".###.", "....." }, // '@'
		{ ".###.", "#...#", "#...#", "#...#", "#####", "#...#", "#...#", "....." }, // 'A'
		{ "####.", "#...#", "#...#", "####.", "#...#", "#...#", "####.", "....." }, // 'B'
		{ ".###.", "#...#", "#....", "#....", "#....", "#...#", ".###.", "....." }, // 'C'
		{ "###..", "#..#.", "#...#", "#...#", "#...#", "#..#.", "###..", "....." }, // 'D'
		{ "#####", "#....", "#....", "####.", "#....", "#....", "#####", "....." }, // 'E'
		{ "#####", "#....", "#....", "####.", "#....", "#....", "#....", "....." }, // 'F'
		{ ".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".####", "....." }, // 'G'
		{ "#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#", "....." }, // 'H'
		{ ".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###.", "....." }, // 'I'
		{ "..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##..", "....." }, // 'J'
		{ "#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#", "....." }, // 'K'
		{ "#....", "#....", "#....", "#....", "#....", "#....", "#####", "....." }, // 'L'
		{ "#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#", "....." }, // 'M'
		{ "#...#", "#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "....." }, // 'N'
		{ ".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###.", "....." }, // 'O'
		{ "####.", "#...#", "#...#", "####.", "#....", "#....", "#....", "....." }, // 'P'
		{ ".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#", "....." }, // 'Q'
		{ "####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#", "....." }, // 'R'
		{ ".####", "#....", "#....", ".###.", "....#", "....#", "####.", "....." }, // 'S'
		{ "#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "....." }, // 'T'
		{ "#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###.", "....." }, // 'U'
		{ "#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#..", "....." }, // 'V'
		{ "#...#", "#...#", "#...#", "#.#.#", "#.#.#", "#.#.#", ".#.#.", "....." }, // 'W'
		{ "#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#", "....." }, // 'X'
		{ "#...#", "#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "....." }, // 'Y'
		{ "#####", "....#", "...#.", "..#..", ".#...", "#....", "#####", "....." }, // 'Z'
		{ ".###.", ".#...", ".#...", ".#...", ".#...", ".#...", ".###.", "....." }, // '['
		{ ".....", "#....", ".#...", "..#..", "...#.", "....#", ".....", "....." }, // backslash
		{ ".###.", "...#.", "...#.", "...#.", "...#.", "...#.", ".###.", "....." }, // ']'
		{ "..#..", ".#.#.", "#...#", ".....", ".....", ".....", ".....", "....." }, // '^'
		{ ".....", ".....", ".....", ".....", ".....", ".....", ".....", "#####" }, // '_'
		{ ".#...", "..#..", "...#.", ".....", ".....", ".....", ".....", "....." }, // '`'
		{ ".....", ".....", ".###.", "....#", ".####", "#...#", ".####", "....." }, // 'a'
		{ "#....", "#....", "#.##.", "##..#", "#...#", "#...#", "####.", "....." }, // 'b'
		{ ".....", ".....", ".###.", "#....", "#....", "#...#", ".###.", "....." }, // 'c'
		{ "....#", "....#", ".##.#", "#..##", "#...#", "#...#", ".####", "....." }, // 'd'
		{ ".....", ".....", ".###.", "#...#", "#####", "#....", ".###.", "....." }, // 'e'
		{ "..##.", ".#..#", ".#...", "###..", ".#...", ".#...", ".#...", "....." }, // 'f'
		{ ".....", ".....", ".####", "#...#", "#...#", ".####", "....#", ".###." }, // 'g'
		{ "#....", "#....", "#.##.", "##..#", "#...#", "#...#", "#...#", "....." }, // 'h'
		{ "..#..", ".....", ".##..", "..#..", "..#..", "..#..", ".###.", "....." }, // 'i'
		{ "...#.", ".....", "..##.", "...#.", "...#.", "...#.", "#..#.", ".##.." }, // 'j'
		{ "#....", "#....", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "....." }, // 'k'
		{ ".##..", "..#..", "..#..", "..#..", "..#..", "..#..", ".###.", "....." }, // 'l'
		{ ".....", ".....", "##.#.", "#.#.#", "#.#.#", "#...#", "#...#", "....." }, // 'm'
		{ ".....", ".....", "#.##.", "##..#", "#...#", "#...#", "#...#", "....." }, // 'n'
		{ ".....", ".....", ".###.", "#...#", "#...#", "#...#", ".###.", "....." }, // 'o'
		{ ".....", ".....", "####.", "#...#", "#...#", "####.", "#....", "#...." }, // 'p'
		{ ".....", ".....", ".####", "#...#", "#...#", ".####", "....#", "....#" }, // 'q'
		{ ".....", ".....", "#.##.", "##..#", "#....", "#....", "#....", "....." }, // 'r'
		{ ".....", ".....", ".###.", "#....", ".###.", "....#", "####.", "....." }, // 's'
		{ ".#...", ".#...", "###..", ".#...", ".#...", ".#..#", "..##.", "....." }, // 't'
		{ ".....", ".....", "#...#", "#...#", "#...#", "#..##", ".##.#", "....." }, // 'u'
		{ ".....", ".....", "#...#", "#...#", "#...#", ".#.#.", "..#..", "....." }, // 'v'
		{ ".....", ".....", "#...#", "#...#", "#.#.#", "#.#.#", ".#.#.", "....." }, // 'w'
		{ ".....", ".....", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "....." }, // 'x'
		{ ".....", ".....", "#...#", "#...#", "#...#", ".####", "....#", ".###." }, // 'y'
		{ ".....", ".....", "#####", "...#.", "..#..", ".#...", "#####", "....." }, // 'z'
		{ "...#.", "..#..", "..#..", ".#...", "..#..", "..#..", "...#.", "....." }, // '{'
		{ "..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "..#..", "....." }, // '|'
		{ ".#...", "..#..", "..#..", "...#.", "..#..", "..#..", ".#...", "....." }, // '}'
		{ ".....", ".....", ".#...", "#.#.#", "...#.", ".....", ".....", "....." }, // '~'
	};

	std::shared_ptr<Font> Font::create_builtin() {
		// Each character gets a cell of 6x10 pixels: its 5x8 pixels, plus an empty column on the
		// right and two empty rows below, so letters and lines don't touch. 16 cells per row.
		const int cell_width = 6;
		const int cell_height = 10;
		const int columns = 16;
		const int glyph_count = int(std::size(builtin_glyphs));
		const int rows = (glyph_count + columns - 1) / columns;
		const int width = columns * cell_width;
		const int height = rows * cell_height;

		// White pixels on a transparent background, so the text's color can tint them
		std::vector<uint8_t> pixels(size_t(width) * height * 4, 0);
		for (int index = 0; index < glyph_count; ++index) {
			int cell_x = (index % columns) * cell_width;
			int cell_y = (index / columns) * cell_height;
			for (int y = 0; y < 8; ++y) {
				for (int x = 0; x < 5; ++x) {
					if (builtin_glyphs[index][y][x] == '#') {
						uint8_t* pixel = &pixels[(size_t(cell_y + y) * width + (cell_x + x)) * 4];
						pixel[0] = pixel[1] = pixel[2] = pixel[3] = 255;
					}
				}
			}
		}

		auto texture = Texture2D::create(width, height, pixels.data());
		if (!texture)
			return nullptr; // The error was already logged
		texture->set_filter(TextureFilter::Nearest); // Pixel letters: keep them sharp at any size

		SpriteSheet cells(texture, cell_width, cell_height);

		// The constructor is private (only these functions make fonts), so std::make_shared can't call it
		std::shared_ptr<Font> font(new Font());
		font->m_line_height = float(cell_height);
		font->m_first_character = ' ';
		font->m_glyphs.resize(glyph_count);
		for (int index = 0; index < glyph_count; ++index) {
			Glyph& glyph = font->m_glyphs[index];
			glyph.sprite = cells.get(index);
			glyph.offset = { 0.0f, 0.0f };                              // Each character fills its cell,
			glyph.size = { float(cell_width), float(cell_height) };     // empty edges included,
			glyph.advance = float(cell_width);                          // and all are equally wide
		}
		return font;
	}

	std::shared_ptr<Font> Font::load(const std::string& path, float pixel_height) {
		if (pixel_height <= 0.0f) {
			MOD_ERROR("Font::load: the pixel height must be above 0, not {}", pixel_height);
			return nullptr;
		}

		std::ifstream file(path, std::ios::binary);
		if (!file) {
			MOD_ERROR("Failed to open font: {}", path);
			return nullptr;
		}
		std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		// stb_truetype barely checks what it reads, so make sure this is a font first
		// (a font starts with a 12-byte table of contents)
		stbtt_fontinfo info;
		int offset = data.size() >= 12 ? stbtt_GetFontOffsetForIndex(data.data(), 0) : -1;
		if (offset < 0 || !stbtt_InitFont(&info, data.data(), offset)) {
			MOD_ERROR("Not a TrueType font: {}", path);
			return nullptr;
		}

		// Draw ("bake") the characters into one image, packed together. If they don't all fit,
		// try again with a bigger image.
		const int first = ' ';
		const int count = 0x100 - first; // Up to U+00FF: ASCII, then Latin-1's accented letters
		std::vector<stbtt_bakedchar> baked(count);
		std::vector<unsigned char> coverage; // How much of each pixel the characters cover, 0 to 255
		int size = 256;
		int rows_used = 0;
		while (true) {
			coverage.assign(size_t(size) * size, 0);
			rows_used = stbtt_BakeFontBitmap(data.data(), offset, pixel_height, coverage.data(), size, size, first, count, baked.data());
			if (rows_used > 0)
				break; // Everything fit (in this many rows of the image)

			if (size >= 4096) {
				MOD_ERROR("Font {} at {} pixels doesn't fit in a 4096x4096 texture", path, pixel_height);
				return nullptr;
			}
			size *= 2;
		}

		// White pixels, as see-through as the characters leave them, so the text's color can tint them.
		// Only the rows that were used.
		const int width = size;
		const int height = rows_used;
		std::vector<uint8_t> pixels(size_t(width) * height * 4);
		for (size_t i = 0; i < size_t(width) * height; ++i) {
			pixels[i * 4 + 0] = pixels[i * 4 + 1] = pixels[i * 4 + 2] = 255;
			pixels[i * 4 + 3] = coverage[i];
		}

		auto texture = Texture2D::create(width, height, pixels.data());
		if (!texture)
			return nullptr; // The error was already logged

		// stb_truetype places characters relative to the baseline (the line letters stand on), so we
		// need to know how far below the top of a line that is
		int ascent = 0, descent = 0, line_gap = 0;
		stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
		float baseline = ascent * stbtt_ScaleForPixelHeight(&info, pixel_height);

		std::shared_ptr<Font> font(new Font());
		font->m_line_height = pixel_height;
		font->m_first_character = char32_t(first);
		font->m_glyphs.resize(count);
		for (int i = 0; i < count; ++i) {
			const stbtt_bakedchar& character = baked[i];
			float glyph_width = float(character.x1 - character.x0);
			float glyph_height = float(character.y1 - character.y0);

			Glyph& glyph = font->m_glyphs[i];
			glyph.sprite = { texture, Rect{ float(character.x0), float(character.y0), glyph_width, glyph_height } };
			glyph.offset = { character.xoff, baseline + character.yoff };
			glyph.size = { glyph_width, glyph_height };
			glyph.advance = character.xadvance;
		}

		MOD_INFO("Loaded font {} ({} pixels, in a {}x{} texture)", path, pixel_height, width, height);
		return font;
	}

	const Font::Glyph& Font::get_glyph(char32_t character) const {
		// Characters 127 to 159 are invisible control codes, not something to draw
		bool control_code = character >= 127 && character < 160;
		if (character < m_first_character || character - m_first_character >= m_glyphs.size() || control_code)
			character = '?';

		return m_glyphs[character - m_first_character];
	}

	glm::vec2 Font::measure(std::string_view text, float height) const {
		float scale = height / m_line_height;
		float line_width = 0.0f;
		float widest = 0.0f;
		int lines = 1;

		for (size_t index = 0; index < text.size();) {
			char32_t character = next_character(text, index);
			if (character == '\n') {
				widest = std::max(widest, line_width);
				line_width = 0.0f;
				++lines;
			} else {
				line_width += get_glyph(character).advance * scale;
			}
		}

		widest = std::max(widest, line_width);
		return { widest, lines * height };
	}

	char32_t Font::next_character(std::string_view text, size_t& index) {
		const char32_t replacement = 0xFFFD;
		unsigned char first = (unsigned char)text[index++];

		// The first byte says how many more belong to this character:
		// 0xxxxxxx: none (ASCII), 110xxxxx: 1, 1110xxxx: 2, 11110xxx: 3. 10xxxxxx only continues one.
		int more = first < 0x80 ? 0 : first >= 0xF0 ? 3 : first >= 0xE0 ? 2 : first >= 0xC0 ? 1 : -1;
		if (more < 0)
			return replacement; // A continuation byte where a character should start

		char32_t character = first & (0x7F >> more); // The bits of the first byte that belong to the character
		for (int i = 0; i < more; ++i) {
			if (index >= text.size() || ((unsigned char)text[index] & 0xC0) != 0x80)
				return replacement; // Cut short
			character = (character << 6) | ((unsigned char)text[index++] & 0x3F); // 6 more bits per byte
		}
		return character;
	}

} // namespace modulus::gfx
