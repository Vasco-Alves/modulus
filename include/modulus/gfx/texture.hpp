#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace modulus::gfx {

	class Texture2D {
	public:
		Texture2D(const std::string& path);
		~Texture2D();

		void bind(uint32_t slot = 0) const;
		void unbind() const;

		int get_width() const { return m_width; }
		int get_height() const { return m_height; }

		// This can be useful for ImGui or raw API access, but keep it generic (uint32_t)
		uint32_t get_renderer_id() const { return m_rendererID; }

		// A static helper to make creating them easy using shared_ptr
		static std::shared_ptr<Texture2D> create(const std::string& path) {
			return std::make_shared<Texture2D>(path);
		}

	private:
		uint32_t m_rendererID = 0;
		std::string m_filePath;
		int m_width = 0, m_height = 0, m_bpp = 0;
	};

}