#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace modulus::gfx {

	// Which part of the 2D world shows up in the window.
	//
	// With the defaults, one unit is one pixel of the window, (0, 0) is the window's top-left
	// corner and y goes down: the same as Input::get_mouse_position(). Change 'position' to
	// scroll the view and 'zoom' to make everything bigger or smaller.
	struct Camera2D {
		glm::vec2 position = { 0.0f, 0.0f }; // The world point at the window's top-left corner
		float zoom = 1.0f;                   // 2 = everything twice as big (must be above 0)

		// Turns a point on the window (like the mouse position) into a point in the world
		glm::vec2 screen_to_world(glm::vec2 screen_point) const {
			return position + screen_point / zoom;
		}

		// Turns a point in the world into a point on the window
		glm::vec2 world_to_screen(glm::vec2 world_point) const {
			return (world_point - position) * zoom;
		}

		// Changes the zoom, but keeps the world point under 'screen_point' in the same place on
		// the window. Pass the mouse position to zoom toward the mouse.
		void zoom_at(glm::vec2 screen_point, float new_zoom) {
			if (new_zoom <= 0.0f)
				return;

			glm::vec2 world_point = screen_to_world(screen_point);
			zoom = new_zoom;
			position = world_point - screen_point / zoom;
		}

		// The matrix that turns world points into OpenGL's -1..1 range, for a window of
		// 'window_size' (in the same units as IWindow::get_size()). Used by the 2D renderer.
		glm::mat4 get_view_projection(glm::vec2 window_size) const {
			// World to window: (world - position) * zoom
			glm::mat4 view = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f))
			               * glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));

			// Window to OpenGL: x from 0..width to -1..1, and y from 0..height to 1..-1
			// (flipped, because y goes down on the window but up in OpenGL)
			glm::mat4 projection = glm::ortho(0.0f, window_size.x, window_size.y, 0.0f, -1.0f, 1.0f);

			return projection * view;
		}
	};

} // namespace modulus::gfx
