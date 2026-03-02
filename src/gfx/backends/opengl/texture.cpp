#include "modulus/gfx/texture.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace modulus::gfx {

	Texture2D::Texture2D(const std::string& path) : m_filePath(path) {
		stbi_set_flip_vertically_on_load(1);

		unsigned char* localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);
		if (!localBuffer) {
			MOD_ERROR("Failed to load texture: {0}", path);
			return;
		}

		glGenTextures(1, &m_rendererID);
		glBindTexture(GL_TEXTURE_2D, m_rendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(localBuffer);
	}

	Texture2D::~Texture2D() {
		glDeleteTextures(1, &m_rendererID);
	}

	void Texture2D::bind(uint32_t slot) const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_rendererID);
	}

	void Texture2D::unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}