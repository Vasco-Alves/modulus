#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace modulus::gfx {

	class Texture2D {
	public:
		// Loads the image. If that fails, the error is logged and is_valid() returns false.
		Texture2D(const std::string& path);

		// Makes a texture from pixels in memory, for textures made in code: width * height pixels
		// of 4 bytes each (red, green, blue, alpha), rows from top to bottom like in an image file.
		Texture2D(int width, int height, const uint8_t* rgba_pixels);
		~Texture2D();

		// Owns a GPU object, so copying is not allowed (both copies would delete it)
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		// False if the image couldn't be loaded
		bool is_valid() const { return m_renderer_id != 0; }

		void bind(uint32_t slot = 0) const;
		void unbind() const;

		int get_width() const { return m_width; }
		int get_height() const { return m_height; }

		// This can be useful for ImGui or raw API access, but keep it generic (uint32_t)
		uint32_t get_renderer_id() const { return m_renderer_id; }

		// Like the constructor, but returns nullptr if the image couldn't be loaded
		// (the error is logged), so you can check it with: if (!texture)
		static std::shared_ptr<Texture2D> create(const std::string& path) {
			auto texture = std::make_shared<Texture2D>(path);
			if (!texture->is_valid())
				return nullptr;

			return texture;
		}

		// The same for a texture made from pixels in memory (nullptr if the size or pixels are invalid)
		static std::shared_ptr<Texture2D> create(int width, int height, const uint8_t* rgba_pixels) {
			auto texture = std::make_shared<Texture2D>(width, height, rgba_pixels);
			if (!texture->is_valid())
				return nullptr;

			return texture;
		}

	private:
		// Creates the OpenGL texture from RGBA pixels ordered bottom row first, the way OpenGL stores them
		void upload(const uint8_t* bottom_up_pixels);

		uint32_t m_renderer_id = 0;
		std::string m_file_path;
		int m_width = 0, m_height = 0, m_bpp = 0;
	};

}