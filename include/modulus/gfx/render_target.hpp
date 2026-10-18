#pragma once

#include <modulus/gfx/color.hpp>
#include <modulus/gfx/texture.hpp>

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

namespace modulus::gfx {

	// An image you can draw into instead of the window, and then draw like a texture.
	//
	// The usual use is a pixel-art game: draw the game at a low resolution (like 320x180) into a
	// render target, then draw that scaled up to fill the window, so every game pixel becomes a
	// big, sharp square (see fit_pixel_perfect below):
	//
	//   low_res->clear(sky);
	//   renderer->begin(*low_res);   // Until end(), everything is drawn into low_res
	//   ...draw the game...
	//   renderer->end();
	//
	//   renderer->begin();           // Back to the window
	//   auto fit = gfx::fit_pixel_perfect({ 320, 180 }, window_size);
	//   renderer->draw_quad(fit.position, fit.size, low_res->get_texture());
	//   renderer->end();
	class RenderTarget {
	public:
		// Returns nullptr if it can't be made (the error is logged). Call gfx::init() first.
		static std::unique_ptr<RenderTarget> create(int width, int height);
		~RenderTarget();

		// Owns GPU objects, so copying is not allowed (both copies would delete them)
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;

		int get_width() const { return m_width; }
		int get_height() const { return m_height; }

		// What was drawn into it, to draw somewhere else. It starts with TextureFilter::Nearest,
		// so its pixels stay sharp when it's drawn bigger.
		Texture2D& get_texture() { return *m_texture; }
		const Texture2D& get_texture() const { return *m_texture; }

		// Fills the whole image with one color
		void clear(const Color& color);

		// From now on, draw into this image instead. unbind() goes back to drawing where things were
		// drawn before. Renderer2D::begin(target) and end() do both for you.
		void bind();
		void unbind();

	private:
		RenderTarget() = default;

		std::shared_ptr<Texture2D> m_texture; // The image itself
		unsigned int m_framebuffer = 0;       // OpenGL's name for "something to draw into"
		unsigned int m_depth_buffer = 0;      // A depth buffer like a window has, so drawing works the same
		int m_width = 0;
		int m_height = 0;

		// What bind() replaced, for unbind() to put back
		int m_previous_framebuffer = 0;
		int m_previous_viewport[4] = {};
	};

	// Where to draw an image in a window so it's as big as possible while every image pixel becomes
	// a whole number of window pixels: pixel art then stays sharp, with all its pixels the same size.
	// The image is centered, with empty bars where it doesn't fill the window. (If the window is
	// smaller than the image, the image is shrunk to fit instead.)
	struct PixelPerfectFit {
		glm::vec2 position; // The image's top-left corner, in the window
		glm::vec2 size;     // The image's size, in the window
		float scale;        // How many window pixels one image pixel becomes

		// Turns a point on the window (like the mouse position) into a point on the image
		glm::vec2 window_to_image(glm::vec2 window_point) const {
			return (window_point - position) / scale;
		}
	};

	inline PixelPerfectFit fit_pixel_perfect(glm::vec2 image_size, glm::vec2 window_size) {
		if (image_size.x <= 0.0f || image_size.y <= 0.0f)
			return { { 0.0f, 0.0f }, { 0.0f, 0.0f }, 1.0f };

		// The biggest scale that fits, rounded down to a whole number (unless even 1 is too big)
		float fits = std::min(window_size.x / image_size.x, window_size.y / image_size.y);
		float scale = fits >= 1.0f ? std::floor(fits) : fits;

		glm::vec2 size = image_size * scale;
		glm::vec2 position = glm::floor((window_size - size) * 0.5f); // Centered, on whole pixels
		return { position, size, scale };
	}

} // namespace modulus::gfx
