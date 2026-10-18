#include "modulus/gfx/sprite_sheet.hpp"
#include "modulus/core/log.hpp"

#include <algorithm>
#include <cmath>

namespace modulus::gfx {

	// =========================================================================
	// SpriteSheet
	// =========================================================================

	SpriteSheet::SpriteSheet(std::shared_ptr<Texture2D> texture, int cell_width, int cell_height, int spacing)
		: m_texture(std::move(texture)), m_cell_width(cell_width), m_cell_height(cell_height), m_spacing(spacing) {
		if (!m_texture || cell_width <= 0 || cell_height <= 0 || spacing < 0) {
			MOD_ERROR("SpriteSheet needs a texture, a cell size above 0, and a spacing of 0 or more");
			return; // No columns or rows, so get() finds no cells
		}

		// How many whole cells fit. Every cell except the last is followed by 'spacing' empty pixels,
		// so count as if there was one more spacing after the last cell too.
		m_columns = (m_texture->get_width() + spacing) / (cell_width + spacing);
		m_rows = (m_texture->get_height() + spacing) / (cell_height + spacing);
	}

	Sprite SpriteSheet::get(int column, int row) const {
		if (column < 0 || column >= m_columns || row < 0 || row >= m_rows) {
			MOD_ERROR("SpriteSheet: there is no cell at column {}, row {} (it has {} columns and {} rows)",
			          column, row, m_columns, m_rows);
			return {}; // No texture, so draw_sprite() draws nothing
		}

		Rect region;
		region.x = float(column * (m_cell_width + m_spacing));
		region.y = float(row * (m_cell_height + m_spacing));
		region.width = float(m_cell_width);
		region.height = float(m_cell_height);
		return { m_texture, region };
	}

	Sprite SpriteSheet::get(int index) const {
		if (index < 0 || index >= get_count()) {
			MOD_ERROR("SpriteSheet: there is no cell number {} (it has {} cells)", index, get_count());
			return {};
		}

		return get(index % m_columns, index / m_columns);
	}

	// =========================================================================
	// SpriteAnimation
	// =========================================================================

	SpriteAnimation::SpriteAnimation(const SpriteSheet& sheet, int first_frame, int frame_count, float frames_per_second, bool looping)
		: m_sheet(sheet), m_first_frame(first_frame), m_frame_count(std::max(frame_count, 1)),
		  m_frames_per_second(frames_per_second), m_looping(looping) {
		if (frame_count < 1 || frames_per_second <= 0.0f)
			MOD_ERROR("SpriteAnimation needs at least 1 frame and more than 0 frames per second");
		else if (first_frame < 0 || first_frame + frame_count > sheet.get_count())
			MOD_ERROR("SpriteAnimation: frames {} to {} don't all exist (the sheet has {} cells)",
			          first_frame, first_frame + frame_count - 1, sheet.get_count());
	}

	void SpriteAnimation::update(float seconds) {
		m_time = std::max(m_time + seconds, 0.0f);

		// A looping animation only needs to know how far it is into the current round. Keeping the
		// time that small also keeps it precise, however long the animation plays.
		if (m_looping && m_frames_per_second > 0.0f) {
			float duration = m_frame_count / m_frames_per_second;
			m_time = std::fmod(m_time, duration);
		}
	}

	void SpriteAnimation::restart() {
		m_time = 0.0f;
	}

	int SpriteAnimation::get_frame() const {
		int frame = int(m_time * m_frames_per_second);
		if (m_looping)
			return frame % m_frame_count;

		return std::min(frame, m_frame_count - 1); // Stay on the last frame
	}

	Sprite SpriteAnimation::get_sprite() const {
		return m_sheet.get(m_first_frame + get_frame());
	}

	bool SpriteAnimation::is_finished() const {
		return !m_looping && m_time * m_frames_per_second >= float(m_frame_count);
	}

} // namespace modulus::gfx
