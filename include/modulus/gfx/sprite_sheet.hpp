#pragma once

#include <modulus/gfx/texture.hpp>

#include <memory>

namespace modulus::gfx {

	// A rectangle on a texture, in its pixels: x and y count from the texture's top-left corner,
	// like in an image editor
	struct Rect {
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	// A part of a texture to draw on its own, like one image of a sprite sheet.
	// Draw it with Renderer2D::draw_sprite(). It keeps its texture alive.
	struct Sprite {
		std::shared_ptr<Texture2D> texture;
		Rect region; // The part of the texture to draw
	};

	// A texture made of equal-sized cells in a grid: a sprite sheet (like the frames of an animation)
	// or a tile set (like the tiles of a level).
	//
	//   gfx::SpriteSheet sheet(texture, 16, 16);  // Cells of 16x16 pixels
	//   renderer->draw_sprite({ 100, 100 }, { 64, 64 }, sheet.get(2, 1)); // Column 2, row 1, drawn 4 times as big
	//
	// Tip: with TextureFilter::Linear (the default), a cell drawn bigger than its size can pick up a
	// little of its neighbors at the edges. Use TextureFilter::Nearest for pixel art, or a sheet with
	// empty pixels between the cells ('spacing').
	class SpriteSheet {
	public:
		// cell_width and cell_height: the size of one cell, in pixels.
		// spacing: how many empty pixels there are between two cells (often 0, sometimes 1 or 2).
		SpriteSheet(std::shared_ptr<Texture2D> texture, int cell_width, int cell_height, int spacing = 0);

		int get_columns() const { return m_columns; }
		int get_rows() const { return m_rows; }
		int get_count() const { return m_columns * m_rows; } // How many cells there are

		// The cell in 'column' and 'row', counting from 0 at the top-left
		Sprite get(int column, int row) const;

		// The cell number 'index', counting from 0 at the top-left, left to right, then row by row
		Sprite get(int index) const;

		const std::shared_ptr<Texture2D>& get_texture() const { return m_texture; }

	private:
		std::shared_ptr<Texture2D> m_texture;
		int m_cell_width;
		int m_cell_height;
		int m_spacing;
		int m_columns = 0;
		int m_rows = 0;
	};

	// Plays cells of a sprite sheet one after another, like a flip book.
	//
	//   gfx::SpriteAnimation walk(sheet, 0, 6, 10.0f); // Cells 0 to 5, 10 of them per second
	//   ...every frame:
	//   walk.update(frame_seconds);
	//   renderer->draw_sprite(position, size, walk.get_sprite());
	class SpriteAnimation {
	public:
		// Plays 'frame_count' cells of 'sheet', starting at cell 'first_frame' (numbered like
		// SpriteSheet::get(index)), 'frames_per_second' cells per second. A looping animation starts
		// over after its last frame; one that doesn't loop stays on its last frame.
		SpriteAnimation(const SpriteSheet& sheet, int first_frame, int frame_count, float frames_per_second, bool looping = true);

		// Moves the animation forward by 'seconds' (usually how long the last frame took)
		void update(float seconds);

		// Back to the first frame
		void restart();

		// The current frame, from 0 to frame_count - 1
		int get_frame() const;

		// The sprite to draw now
		Sprite get_sprite() const;

		// True once an animation that doesn't loop has shown its last frame for its full time
		bool is_finished() const;

	private:
		SpriteSheet m_sheet;
		int m_first_frame;
		int m_frame_count;
		float m_frames_per_second;
		bool m_looping;
		float m_time = 0.0f; // Seconds since the start (for looping animations: since the last start over)
	};

} // namespace modulus::gfx
