#pragma once

#include <modulus/gfx/camera_2d.hpp>
#include <modulus/gfx/color.hpp>
#include <modulus/platform/window.hpp>

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>

namespace modulus::gfx {

	class Shader;
	class Texture2D;
	class VertexArray;
	class VertexBuffer;

	// Draws rectangles ("quads") with a color or a texture (sprites), fast: it collects everything
	// drawn between begin() and end() and sends it to the GPU in as few draw calls as possible,
	// instead of one draw call per rectangle.
	//
	//   renderer->begin(camera);
	//   renderer->draw_quad({ 10, 10 }, { 100, 50 }, { 1.0f, 0.0f, 0.0f, 1.0f }); // A red rectangle
	//   renderer->draw_quad({ 200, 10 }, { 64, 64 }, *player_texture);            // A sprite
	//   renderer->end();
	//
	// Positions and sizes are in world units (see Camera2D): with the default camera, window pixels
	// with (0, 0) at the top-left corner. A quad's position is its top-left corner.
	// Quads are drawn in order, so later ones cover earlier ones.
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

		void draw_quad(glm::vec2 position, glm::vec2 size, const Color& color);

		// 'tint' multiplies the texture's colors: white keeps them as they are
		void draw_quad(glm::vec2 position, glm::vec2 size, const Texture2D& texture,
		               const Color& tint = { 1.0f, 1.0f, 1.0f, 1.0f });

		// Like draw_quad(), but turned 'degrees' clockwise around the quad's center
		void draw_rotated_quad(glm::vec2 position, glm::vec2 size, float degrees, const Color& color);
		void draw_rotated_quad(glm::vec2 position, glm::vec2 size, float degrees, const Texture2D& texture,
		                       const Color& tint = { 1.0f, 1.0f, 1.0f, 1.0f });

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

		void add_quad(glm::vec2 position, glm::vec2 size, float degrees, const Texture2D* texture, const Color& color);
		int find_texture_slot(const Texture2D* texture); // -1 if the batch has no free slot left
		void flush(); // Draws the quads collected so far and starts an empty batch

		platform::IWindow& m_window;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<VertexBuffer> m_vertex_buffer;
		std::shared_ptr<VertexArray> m_vertex_array;
		std::shared_ptr<Texture2D> m_white_texture; // Plain-colored quads "use" this texture, in slot 0

		// The current batch: its vertices and the textures they use
		std::vector<QuadVertex> m_vertices;
		std::array<const Texture2D*, max_textures_per_draw_call> m_texture_slots = {};
		int m_texture_slot_count = 1;                             // Slot 0 always holds the white texture
		int m_texture_slot_limit = max_textures_per_draw_call; // Lower if the GPU has fewer texture units

		Stats m_stats;
	};

} // namespace modulus::gfx
