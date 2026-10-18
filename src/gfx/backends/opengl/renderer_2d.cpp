#include "modulus/gfx/renderer_2d.hpp"
#include "modulus/gfx/buffer.hpp"
#include "modulus/gfx/graphics.hpp"
#include "modulus/gfx/shader.hpp"
#include "modulus/gfx/texture.hpp"
#include "modulus/gfx/vertex_array.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace modulus::gfx {

	// Each corner brings its position, color, texture coordinate and texture slot.
	// The camera's matrix turns world positions into OpenGL's -1..1 range.
	static const char* vertex_shader_source = R"(#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_TextureSlot;

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
flat out int v_TextureSlot; // "flat": the same for the whole quad, not blended between corners

void main() {
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TextureSlot = a_TextureSlot;
	gl_Position = u_ViewProjection * vec4(a_Position, 0.0, 1.0);
}
)";

	// Each pixel is the texture's color times the quad's color.
	// Plain-colored quads use a white texture, so they come out in exactly their color.
	static const char* fragment_shader_source = R"(#version 330 core
in vec4 v_Color;
in vec2 v_TexCoord;
flat in int v_TextureSlot;

uniform sampler2D u_Textures[16];

layout(location = 0) out vec4 o_Color;

// GLSL 3.30 only allows constant indices into an array of textures, hence the switch
vec4 sample_texture(int slot, vec2 uv) {
	switch (slot) {
	case 0:  return texture(u_Textures[0], uv);
	case 1:  return texture(u_Textures[1], uv);
	case 2:  return texture(u_Textures[2], uv);
	case 3:  return texture(u_Textures[3], uv);
	case 4:  return texture(u_Textures[4], uv);
	case 5:  return texture(u_Textures[5], uv);
	case 6:  return texture(u_Textures[6], uv);
	case 7:  return texture(u_Textures[7], uv);
	case 8:  return texture(u_Textures[8], uv);
	case 9:  return texture(u_Textures[9], uv);
	case 10: return texture(u_Textures[10], uv);
	case 11: return texture(u_Textures[11], uv);
	case 12: return texture(u_Textures[12], uv);
	case 13: return texture(u_Textures[13], uv);
	case 14: return texture(u_Textures[14], uv);
	case 15: return texture(u_Textures[15], uv);
	}
	return vec4(1.0);
}

void main() {
	o_Color = sample_texture(v_TextureSlot, v_TexCoord) * v_Color;
}
)";

	std::unique_ptr<Renderer2D> Renderer2D::create(platform::IWindow& window) {
		// The constructor is private (only create() makes renderers), so std::make_unique can't call it
		std::unique_ptr<Renderer2D> renderer(new Renderer2D(window));
		if (!renderer->m_shader)
			return nullptr; // The shader's error was already logged

		return renderer;
	}

	Renderer2D::Renderer2D(platform::IWindow& window) : m_window(window) {
		m_shader = Shader::create(vertex_shader_source, fragment_shader_source);
		if (!m_shader)
			return; // create() sees this and returns nullptr

		// Texture slot i of a batch is bound to texture unit i
		static_assert(max_textures_per_draw_call == 16, "The fragment shader handles exactly 16 texture slots");
		int units[max_textures_per_draw_call];
		for (int i = 0; i < max_textures_per_draw_call; ++i)
			units[i] = i;
		m_shader->set_int_array("u_Textures", units, max_textures_per_draw_call);

		// OpenGL 3.3 promises at least 16 texture units, but ask anyway
		GLint available_units = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &available_units);
		m_texture_slot_limit = std::min<int>(max_textures_per_draw_call, available_units);

		// The vertex buffer is refilled with the corners of each batch
		m_vertex_buffer = std::make_shared<VertexBuffer>(max_quads_per_draw_call * 4 * sizeof(QuadVertex));
		m_vertex_array = std::make_shared<VertexArray>();
		m_vertex_array->add_vertex_buffer(m_vertex_buffer, {
			{ ShaderDataType::Float2, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Int, "a_TextureSlot" },
		});
		static_assert(sizeof(QuadVertex) == 36, "QuadVertex must match the layout above");

		// Every quad is two triangles over its 4 corners, so the indices never change:
		// 0-1-2 and 2-3-0 for the first quad, 4-5-6 and 6-7-4 for the second, and so on
		std::vector<unsigned int> indices(max_quads_per_draw_call * 6);
		for (unsigned int quad = 0; quad < max_quads_per_draw_call; ++quad) {
			unsigned int first_corner = quad * 4;
			unsigned int* triangles = &indices[quad * 6];
			triangles[0] = first_corner + 0;
			triangles[1] = first_corner + 1;
			triangles[2] = first_corner + 2;
			triangles[3] = first_corner + 2;
			triangles[4] = first_corner + 3;
			triangles[5] = first_corner + 0;
		}
		m_vertex_array->set_index_buffer(std::make_shared<IndexBuffer>(indices.data(), (unsigned int)indices.size()));

		const uint8_t white[4] = { 255, 255, 255, 255 };
		m_white_texture = Texture2D::create(1, 1, white);
		m_texture_slots[0] = m_white_texture.get();

		m_vertices.reserve(max_quads_per_draw_call * 4);
	}

	Renderer2D::~Renderer2D() = default;

	void Renderer2D::begin(const Camera2D& camera) {
		// Quads left from a begin() without end(): draw them, with the camera they were meant for
		if (!m_vertices.empty()) {
			MOD_WARN("Renderer2D::begin() was called again before end(), so the quads from before are drawn now");
			flush();
		}

		m_stats = {};

		// The window size is in the same units as the mouse position. (On high-DPI screens the window
		// can have more pixels than that, which just makes everything sharper.)
		// Never 0, so the math still works while the window is minimized.
		auto [width, height] = m_window.get_size();
		glm::vec2 window_size = { float(std::max(width, 1)), float(std::max(height, 1)) };
		m_shader->set_mat4("u_ViewProjection", camera.get_view_projection(window_size));
	}

	// Textures are drawn in their own colors, unless DrawOptions::tint changes them
	static const Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

	void Renderer2D::draw_quad(glm::vec2 position, glm::vec2 size, const Color& color, const DrawOptions& options) {
		add_quad(position, size, nullptr, color, options);
	}

	void Renderer2D::draw_quad(glm::vec2 position, glm::vec2 size, const Texture2D& texture, const DrawOptions& options) {
		add_quad(position, size, &texture, white, options);
	}

	void Renderer2D::draw_sprite(glm::vec2 position, glm::vec2 size, const Sprite& sprite, const DrawOptions& options) {
		if (!sprite.texture || !sprite.texture->is_valid())
			return; // An empty sprite, for example from SpriteSheet::get() with a cell that doesn't exist

		// The region is in pixels from the image's top-left, but texture coordinates go from 0 to 1,
		// with the image's top at 1 (OpenGL stores images bottom row first)
		float width = float(sprite.texture->get_width());
		float height = float(sprite.texture->get_height());
		const Rect& region = sprite.region;
		glm::vec4 tex_coords = {
			region.x / width,                           // Left
			1.0f - region.y / height,                   // Top
			(region.x + region.width) / width,          // Right
			1.0f - (region.y + region.height) / height, // Bottom
		};

		add_quad(position, size, sprite.texture.get(), white, options, tex_coords);
	}

	void Renderer2D::end() {
		flush();
	}

	void Renderer2D::add_quad(glm::vec2 position, glm::vec2 size, const Texture2D* texture, const Color& color,
	                          const DrawOptions& options, glm::vec4 tex_coords) {
		// The batch is full: draw it, and continue with an empty one
		if (m_vertices.size() == max_quads_per_draw_call * 4)
			flush();

		int slot = 0; // The white texture, for plain colors
		if (texture) {
			slot = find_texture_slot(texture);
			if (slot < 0) {
				// Every texture slot is taken: draw the batch, then there's room again
				flush();
				slot = find_texture_slot(texture);
			}
		}

		// The corners, clockwise from the top-left (y goes down), measured from the origin
		glm::vec2 origin = options.origin * size;
		glm::vec2 corners[4] = {
			glm::vec2(0.0f, 0.0f) - origin,
			glm::vec2(size.x, 0.0f) - origin,
			size - origin,
			glm::vec2(0.0f, size.y) - origin,
		};

		// Turn them around the origin. Because y goes down, a positive angle turns clockwise.
		if (options.rotation != 0.0f) {
			float cosine = std::cos(glm::radians(options.rotation));
			float sine = std::sin(glm::radians(options.rotation));
			for (glm::vec2& corner : corners)
				corner = glm::vec2(corner.x * cosine - corner.y * sine, corner.x * sine + corner.y * cosine);
		}

		// Then put the origin at 'position'
		for (glm::vec2& corner : corners)
			corner += position;

		// Mirroring the texture just swaps the sides it is read from
		if (options.flip_x)
			std::swap(tex_coords.x, tex_coords.z); // Left <-> right
		if (options.flip_y)
			std::swap(tex_coords.y, tex_coords.w); // Top <-> bottom

		// Each corner's texture coordinate, in the same order as the corners
		const glm::vec2 corner_tex_coords[4] = {
			{ tex_coords.x, tex_coords.y }, // Left, top
			{ tex_coords.z, tex_coords.y }, // Right, top
			{ tex_coords.z, tex_coords.w }, // Right, bottom
			{ tex_coords.x, tex_coords.w }, // Left, bottom
		};
		const Color& tint = options.tint;
		glm::vec4 rgba = { color.r * tint.r, color.g * tint.g, color.b * tint.b, color.a * tint.a };

		for (int i = 0; i < 4; ++i)
			m_vertices.push_back({ corners[i], rgba, corner_tex_coords[i], slot });
	}

	int Renderer2D::find_texture_slot(const Texture2D* texture) {
		// Already used in this batch?
		for (int slot = 1; slot < m_texture_slot_count; ++slot) {
			if (m_texture_slots[slot] == texture)
				return slot;
		}

		if (m_texture_slot_count == m_texture_slot_limit)
			return -1;

		m_texture_slots[m_texture_slot_count] = texture;
		return m_texture_slot_count++;
	}

	void Renderer2D::flush() {
		if (m_vertices.empty())
			return;

		m_vertex_buffer->set_data(m_vertices.data(), m_vertices.size() * sizeof(QuadVertex));

		for (int slot = 0; slot < m_texture_slot_count; ++slot)
			m_texture_slots[slot]->bind(slot);

		// Quads are drawn in order, each on top of the ones before. The depth test would hide a quad
		// behind an earlier one at the same depth instead, so it's off while drawing (and restored after).
		GLboolean depth_test_was_on = glIsEnabled(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);

		auto quad_count = (unsigned int)(m_vertices.size() / 4);
		m_shader->bind();
		draw_indexed(*m_vertex_array, quad_count * 6);

		if (depth_test_was_on)
			glEnable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0); // Leave texture unit 0 selected, as most code expects

		m_stats.quads += quad_count;
		m_stats.draw_calls++;

		// Start an empty batch, with only the white texture in slot 0
		m_vertices.clear();
		m_texture_slot_count = 1;
	}

} // namespace modulus::gfx
