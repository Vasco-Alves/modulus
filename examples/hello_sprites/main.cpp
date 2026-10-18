// Hello Sprites: sprite sheets and animations.
//   Walk with the arrow keys or WASD and collect the spinning coins.
//   Escape: quit.
//
// The sprite sheets are drawn in code, so this example needs no image files.
// A real game would load them from PNG files instead:
//   auto tile_set = gfx::Texture2D::create(core::File::app_path("assets/tiles.png"));

#include <modulus/core/timer.hpp>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/renderer_2d.hpp>
#include <modulus/gfx/sprite_sheet.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <vector>

using namespace modulus;
using platform::Input;
using platform::Key;

// The cells of the tile set, in order
enum Tile { Grass, Water, Sand, Stone, Player };

// A picture in memory, to draw the sprite sheets into
struct Image {
	int width, height;
	std::vector<uint8_t> pixels; // Red, green, blue, alpha for each pixel. Starts transparent.

	Image(int width, int height) : width(width), height(height), pixels(size_t(width) * height * 4, 0) {}

	void set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
		uint8_t* pixel = &pixels[(size_t(y) * width + x) * 4];
		pixel[0] = r; pixel[1] = g; pixel[2] = b; pixel[3] = 255;
	}

	std::shared_ptr<gfx::Texture2D> to_texture() const {
		auto texture = gfx::Texture2D::create(width, height, pixels.data());
		texture->set_filter(gfx::TextureFilter::Nearest); // Pixel art: keep the pixels sharp
		return texture;
	}
};

// The tile set: 5 cells of 8x8 pixels in a row
static std::shared_ptr<gfx::Texture2D> make_tile_set() {
	Image image(5 * 8, 8);
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			bool speck = (x * 7 + y * 3) % 5 == 0;
			image.set(Grass * 8 + x, y, 60, speck ? 140 : 170, 60);
			image.set(Water * 8 + x, y, 40, 90, (y + x / 3) % 4 == 0 ? 235 : 200);
			image.set(Sand * 8 + x, y, 220, 200, speck ? 120 : 150);
			image.set(Stone * 8 + x, y, speck ? 95 : 135, speck ? 95 : 135, speck ? 100 : 140);

			// The player: a round yellow face looking to the right, on a transparent background
			bool eye = y == 2 && (x == 3 || x == 6);
			bool mouth = y == 5 && x >= 3 && x <= 6;
			bool face = (x - 3.5f) * (x - 3.5f) + (y - 3.5f) * (y - 3.5f) <= 13.0f;
			if (eye || mouth)
				image.set(Player * 8 + x, y, 40, 30, 20);
			else if (face)
				image.set(Player * 8 + x, y, 250, 210, 60);
		}
	}
	return image.to_texture();
}

// The coin animation: 6 cells of 16x16 pixels. The coin turns, so it looks thinner, then wider again.
static std::shared_ptr<gfx::Texture2D> make_coin_sheet() {
	Image image(6 * 16, 16);
	for (int frame = 0; frame < 6; ++frame) {
		float half_width = std::max(1.0f, 6.5f * std::abs(std::cos(frame * 3.14159f / 6.0f)));
		for (int y = 0; y < 16; ++y) {
			for (int x = 0; x < 16; ++x) {
				float dx = (x + 0.5f - 8.0f) / half_width;
				float dy = (y + 0.5f - 8.0f) / 6.5f;
				float distance = dx * dx + dy * dy; // Below 1: inside the coin
				if (distance <= 1.0f) {
					bool rim = distance > 0.55f;
					image.set(frame * 16 + x, y, rim ? 205 : 255, rim ? 150 : 205, 40);
				}
			}
		}
	}
	return image.to_texture();
}

// Which tile goes where: water around the edges, then a strip of sand, then grass with a few stones
static Tile tile_at(int x, int y, int columns, int rows) {
	int from_edge = std::min({ x, y, columns - 1 - x, rows - 1 - y });
	if (from_edge < 2)
		return Water;
	if (from_edge < 3)
		return Sand;
	if ((x * 31 + y * 17) % 11 == 0)
		return Stone;
	return Grass;
}

int main() {
	platform::WindowConfig config;
	config.title = "Modulus - Hello Sprites";

	auto window = platform::create_window(config);
	if (!window || !gfx::init(*window))
		return -1;

	auto renderer = gfx::Renderer2D::create(*window);
	if (!renderer)
		return -1;

	gfx::SpriteSheet tiles(make_tile_set(), 8, 8);
	gfx::SpriteSheet coin_sheet(make_coin_sheet(), 16, 16);

	const float tile_size = 32.0f; // The 8x8 tiles are drawn 4 times as big
	const int columns = 40, rows = 23;  // Enough to fill the 1280x720 window

	struct Coin {
		glm::vec2 center;
		gfx::SpriteAnimation animation;
		bool collected = false;
	};
	std::vector<Coin> coins;
	for (int i = 0; i < 12; ++i) {
		Coin coin = { { 220.0f + (i % 6) * 170.0f, 200.0f + (i / 6) * 300.0f }, gfx::SpriteAnimation(coin_sheet, 0, 6, 10.0f) };
		coin.animation.update(i * 0.05f); // So the coins don't all turn in step
		coins.push_back(coin);
	}

	glm::vec2 player = { 640.0f, 360.0f };
	bool facing_left = false;
	int collected = 0;
	core::Timer frame_timer;

	while (window->update()) {
		// How long the last frame took (at most a tenth of a second, so nothing jumps after a pause)
		float seconds = std::min(frame_timer.elapsed(), 0.1f);
		frame_timer.reset();

		if (Input::is_key_pressed(Key::Escape))
			window->close();

		// Walk with the arrow keys or WASD
		glm::vec2 direction = { 0.0f, 0.0f };
		if (Input::is_key_down(Key::Left) || Input::is_key_down(Key::A)) direction.x -= 1.0f;
		if (Input::is_key_down(Key::Right) || Input::is_key_down(Key::D)) direction.x += 1.0f;
		if (Input::is_key_down(Key::Up) || Input::is_key_down(Key::W)) direction.y -= 1.0f;
		if (Input::is_key_down(Key::Down) || Input::is_key_down(Key::S)) direction.y += 1.0f;
		if (direction.x != 0.0f || direction.y != 0.0f)
			player += glm::normalize(direction) * 250.0f * seconds;

		// Face the way we last walked (the sprite looks right, so flip it to look left)
		if (direction.x < 0.0f)
			facing_left = true;
		else if (direction.x > 0.0f)
			facing_left = false;

		for (Coin& coin : coins) {
			coin.animation.update(seconds);

			if (!coin.collected && glm::distance(coin.center, player) < 32.0f) {
				coin.collected = true;
				window->set_title(std::format("Modulus - Hello Sprites ({} of {} coins)", ++collected, coins.size()));
			}
		}

		gfx::clear({ 0.0f, 0.0f, 0.0f, 1.0f });
		renderer->begin();

		for (int y = 0; y < rows; ++y) {
			for (int x = 0; x < columns; ++x) {
				glm::vec2 position = { x * tile_size, y * tile_size };
				renderer->draw_sprite(position, { tile_size, tile_size }, tiles.get(tile_at(x, y, columns, rows)));
			}
		}

		// The coins and the player are placed by their center
		for (const Coin& coin : coins) {
			if (!coin.collected)
				renderer->draw_sprite(coin.center, { 32.0f, 32.0f }, coin.animation.get_sprite(), { .origin = { 0.5f, 0.5f } });
		}

		renderer->draw_sprite(player, { 40.0f, 40.0f }, tiles.get(Player), { .origin = { 0.5f, 0.5f }, .flip_x = facing_left });

		renderer->end();
		gfx::present(*window);
	}

	return 0;
}
