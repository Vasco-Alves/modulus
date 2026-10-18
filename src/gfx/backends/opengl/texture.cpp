#include "modulus/gfx/texture.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

#include <cstring>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace modulus::gfx {

	Texture2D::Texture2D(const std::string& path) : m_file_path(path) {
		// Image files start with the top row, but OpenGL expects the bottom row first
		stbi_set_flip_vertically_on_load(1);

		unsigned char* pixels = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);
		if (!pixels) {
			MOD_ERROR("Failed to load texture: {0} ({1})", path, stbi_failure_reason());

			// stb_image may have filled these in before it failed. m_renderer_id stays 0, so is_valid() is false.
			m_width = m_height = m_bpp = 0;
			return;
		}

		upload(pixels);
		stbi_image_free(pixels);
	}

	Texture2D::Texture2D(int width, int height, const uint8_t* rgba_pixels) {
		if (width <= 0 || height <= 0 || !rgba_pixels) {
			MOD_ERROR("Can't make a texture of {}x{} pixels{}", width, height, rgba_pixels ? "" : " without pixel data");
			return;
		}

		m_width = width;
		m_height = height;
		m_bpp = 4;

		// Flip the rows (top row first -> bottom row first), like stb_image does for files
		const size_t row_size = size_t(width) * 4;
		std::vector<uint8_t> flipped(row_size * height);
		for (int y = 0; y < height; ++y)
			std::memcpy(&flipped[size_t(height - 1 - y) * row_size], &rgba_pixels[size_t(y) * row_size], row_size);

		upload(flipped.data());
	}

	void Texture2D::upload(const uint8_t* bottom_up_pixels) {
		glGenTextures(1, &m_renderer_id);
		glBindTexture(GL_TEXTURE_2D, m_renderer_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bottom_up_pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture2D::~Texture2D() {
		glDeleteTextures(1, &m_renderer_id);
	}

	void Texture2D::bind(uint32_t slot) const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_renderer_id);
	}

	void Texture2D::unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}