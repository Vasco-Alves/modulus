#pragma once

#include <modulus/platform/window.hpp>
#include <modulus/gfx/vertex_array.hpp>

namespace modulus::gfx {

	struct Color {
		float r, g, b, a;
	};

	// Starts the graphics module.
	// Graphics draws into a window, so create the window first and pass it here.
	//
	// There is no shutdown(): OpenGL is cleaned up when the window is destroyed.
	bool init(platform::IWindow& window);

	// Set the viewport (usually called when window resizes)
	void set_viewport(int x, int y, int width, int height);

	void clear(const Color& color);

	void draw_indexed(const std::shared_ptr<VertexArray>& vertexArray);

	// Shows the finished frame in the window. Call it once at the end of every frame.
	void present(platform::IWindow& window);

} // namespace modulus::gfx