#pragma once

#include <modulus/gfx/camera_2d.hpp>
#include <modulus/gfx/color.hpp>
#include <modulus/gfx/font.hpp>
#include <modulus/gfx/sprite_sheet.hpp>
#include <modulus/platform/window.hpp>

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <string_view>
#include <vector>

namespace modulus::gfx {

	class Shader;
	class Texture2D;
	class VertexArray;
	class VertexBuffer;

	// Optional settings for Renderer2D's draw functions. Name only the ones you need, in this order:
	//   renderer->draw_sprite(position, size, sprite, { .origin = { 0.5f, 0.5f }, .rotation = 45.0f });
	struct DrawOptions {
		// The point of the quad that 'position' refers to, and that it turns around:
		// (0, 0) is its top-left corner (the default), (0.5, 0.5) its center, and (0.5, 1) the
		// middle of its bottom edge (handy for a character's feet)
		glm::vec2 origin = { 0.0f, 0.0f };

		float rotation = 0.0f; // In degrees, clockwise, around the origin

		bool flip_x = false; // Mirror the texture left to right (like a character turning around)
		bool flip_y = false; // Mirror the texture top to bottom

		Color tint = { 1.0f, 1.0f, 1.0f, 1.0f }; // Multiplies the colors: white keeps them as they are
	};

	// Draws rectangles ("quads") with a color, a texture, or part of a texture (a sprite), fast:
	// it collects everything drawn between begin() and end() and sends it to the GPU in as few
	// draw calls as possible, instead of one draw call per rectangle.
	//
	//   renderer->begin(camera);
	//   renderer->draw_quad({ 10, 10 }, { 100, 50 }, { 1.0f, 0.0f, 0.0f, 1.0f }); // A red rectangle
	//   renderer->draw_quad({ 200, 10 }, { 64, 64 }, *texture);                   // A whole texture
	//   renderer->draw_sprite({ 300, 10 }, { 64, 64 }, sheet.get(3));             // One cell of a sprite sheet
	//   renderer->end();
	//
	// Positions and sizes are in world units (see Camera2D): with the default camera, window pixels
	// with (0, 0) at the top-left corner. 'position' is where the quad's origin goes, which is its
	// top-left corner unless DrawOptions says otherwise. Quads are drawn in order, so later ones
	// cover earlier ones.
	class Renderer2D {
	public:
		// Call gfx::init() first. Returns nullptr if the renderer can't be set up (the error is logged).
		// The renderer draws into 'window', so the window must outlive it.
		static std::unique_ptr<Renderer2D> create(platform::IWindow& window);
		~Renderer2D();

		// Owns GPU objects, so copying is not allowed (both copies would delete them)
		Renderer2D(const Renderer2D&) = delete;
		Renderer2D& operator=(const Renderer2D&) = delete;

		// Starts collecting quads, seen through 'camera'
		void begin(const Camera2D& camera = {});

		void draw_quad(glm::vec2 position, glm::vec2 size, const Color& color, const DrawOptions& options = {});
		void draw_quad(glm::vec2 position, glm::vec2 size, const Texture2D& texture, const DrawOptions& options = {});

		// Draws a sprite: one part of a texture, like a cell of a SpriteSheet.
		// An empty sprite (no texture) draws nothing.
		void draw_sprite(glm::vec2 position, glm::vec2 size, const Sprite& sprite, const DrawOptions& options = {});

		// Draws text with the built-in pixel font. 'height' is the height of one line, and '\n'
		// starts a new line. DrawOptions::origin is measured on the whole text, so { 0.5f, 0.5f }
		// centers the text on 'position', and rotation turns the text as a whole.
		void draw_text(glm::vec2 position, std::string_view text, float height, const Color& color,
		               const DrawOptions& options = {});

		// The same with another font
		void draw_text(glm::vec2 position, std::string_view text, const Font& font, float height,
		               const Color& color, const DrawOptions& options = {});

		// The built-in pixel font, for example to measure text: get_builtin_font().measure("Score", 20.0f)
		const Font& get_builtin_font() const { return *m_builtin_font; }

		// Draws everything collected since begin(). Textures must stay alive until then.
		void end();

		// What the last begin() ... end() drew
		struct Stats {
			unsigned int quads = 0;
			unsigned int draw_calls = 0;
		};
		const Stats& get_stats() const { return m_stats; }

		static constexpr unsigned int max_quads_per_draw_call = 10000;
		static constexpr int max_textures_per_draw_call = 16; // Including a white one for plain colors

	private:
		explicit Renderer2D(platform::IWindow& window);

		// One corner of a quad, the way the shader receives it
		struct QuadVertex {
			glm::vec2 position;
			glm::vec4 color;
			glm::vec2 tex_coord;
			int texture_slot; // Which of the batch's textures to use
		};

		// tex_coords: the part of the texture to show, as (left, top, right, bottom) in OpenGL's 0 to 1
		// texture coordinates. The default is the whole texture: its top is at 1, because OpenGL
		// stores images bottom row first.
		void add_quad(glm::vec2 position, glm::vec2 size, const Texture2D* texture, const Color& color,
		              const DrawOptions& options, glm::vec4 tex_coords = { 0.0f, 1.0f, 1.0f, 0.0f });
		int find_texture_slot(const Texture2D* texture); // -1 if the batch has no free slot left
		void flush(); // Draws the quads collected so far and starts an empty batch

		platform::IWindow& m_window;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<VertexBuffer> m_vertex_buffer;
		std::shared_ptr<VertexArray> m_vertex_array;
		std::shared_ptr<Texture2D> m_white_texture; // Plain-colored quads "use" this texture, in slot 0
		std::shared_ptr<Font> m_builtin_font;

		// The current batch: its vertices and the textures they use
		std::vector<QuadVertex> m_vertices;
		std::array<const Texture2D*, max_textures_per_draw_call> m_texture_slots = {};
		int m_texture_slot_count = 1;                          // Slot 0 always holds the white texture
		int m_texture_slot_limit = max_textures_per_draw_call; // Lower if the GPU has fewer texture units

		Stats m_stats;
	};

} // namespace modulus::gfx
