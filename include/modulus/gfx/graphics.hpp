#pragma once

#include <modulus/platform/window.hpp>
#include <modulus/gfx/vertex_array.hpp>

namespace modulus::gfx {

	struct Color {
		float r, g, b, a;
	};

	// Initialize the Graphics Backend (Loads OpenGL functions via GLAD)
	bool init();

	// Shutdown graphics resources
	void shutdown();

	// Set the viewport (usually called when window resizes)
	void set_viewport(int x, int y, int width, int height);

	// Clear the screen to a specific color
	void clear(const Color& color);

	void draw_indexed(const std::shared_ptr<VertexArray>& vertexArray);

	// Swap buffers (Present the frame to the user)
	// We pass the window because in OpenGL, the swap is tied to the window handle.
	void present(platform::IWindow& window);

} // namespace modulus::gfx