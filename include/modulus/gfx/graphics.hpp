#pragma once

#include <modulus/platform/window.hpp>
#include <modulus/gfx/color.hpp>
#include <modulus/gfx/vertex_array.hpp>

namespace modulus::gfx {

	// Starts the graphics module.
	// Graphics draws into a window, so create the window first and pass it here.
	// From then on, drawing covers the whole window, also after it is resized.
	// In debug builds, OpenGL errors are logged as they happen (if the driver supports it).
	//
	// There is no shutdown(): OpenGL is cleaned up when the window is destroyed.
	// So destroy graphics objects (shaders, buffers, textures, ...) before the window: they need its
	// OpenGL context to free their GPU memory. In main(), that happens by itself when the window is created first.
	bool init(platform::IWindow& window);

	// Draw into part of the window only (x and y from the bottom-left corner, in pixels).
	// Resizing the window resets it to the whole window.
	void set_viewport(int x, int y, int width, int height);

	void clear(const Color& color);

	void draw_indexed(const std::shared_ptr<VertexArray>& vertexArray);

	// Shows the finished frame in the window. Call it once at the end of every frame.
	void present(platform::IWindow& window);

} // namespace modulus::gfx