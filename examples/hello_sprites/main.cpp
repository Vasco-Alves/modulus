// Hello Sprites: a little pixel-art game with sprite sheets, animations and a sound.
//   Walk with the arrow keys or WASD and collect the spinning coins.
//   Resize the window: the game stays sharp, scaled by whole numbers.
//   Escape: quit.
//
// The game is drawn at 320x180 pixels into a render target, then shown scaled up in the window.
//
// The sprite sheets are drawn in code, so this example needs no image files.
// A real game would load them from PNG files instead:
//   auto tile_set = gfx::Texture2D::create(core::File::app_path("assets/tiles.png"));
// The coin sound is a file, though: assets/sounds/coin.wav, copied next to the program by CMake.

#include <modulus/core/file.hpp>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/render_target.hpp>
#include <modulus/gfx/renderer_2d.hpp>
#include <modulus/gfx/sprite_sheet.hpp>

#include <modulus/audio/audio.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
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

// The coin animation: 6 cells of 10x10 pixels. The coin turns, so it looks thinner, then wider again.
static std::shared_ptr<gfx::Texture2D> make_coin_sheet() {
	Image image(6 * 10, 10);
	for (int frame = 0; frame < 6; ++frame) {
		float half_width = std::max(1.0f, 4.5f * std::abs(std::cos(frame * 3.14159f / 6.0f)));
		for (int y = 0; y < 10; ++y) {
			for (int x = 0; x < 10; ++x) {
				float dx = (x + 0.5f - 5.0f) / half_width;
				float dy = (y + 0.5f - 5.0f) / 4.5f;
				float distance = dx * dx + dy * dy; // Below 1: inside the coin
				if (distance <= 1.0f) {
					bool rim = distance > 0.55f;
					image.set(frame * 10 + x, y, rim ? 205 : 255, rim ? 150 : 205, 40);
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

	// The game is drawn into this 320x180 image, in which one unit is one pixel.
	// It's then shown scaled up in the window: 4 times at the window's starting size of 1280x720.
	const glm::vec2 game_size = { 320.0f, 180.0f };
	auto game_screen = gfx::RenderTarget::create(int(game_size.x), int(game_size.y));
	if (!game_screen)
		return -1;

	gfx::SpriteSheet tiles(make_tile_set(), 8, 8);
	gfx::SpriteSheet coin_sheet(make_coin_sheet(), 10, 10);

	const float tile_size = 8.0f;
	const int columns = 40, rows = 23; // Enough tiles to fill the game screen

	struct Coin {
		glm::vec2 center;
		gfx::SpriteAnimation animation;
		bool collected = false;
	};
	std::vector<Coin> coins;
	for (int i = 0; i < 12; ++i) {
		Coin coin = { { 55.0f + (i % 6) * 42.0f, 50.0f + (i / 6) * 75.0f }, gfx::SpriteAnimation(coin_sheet, 0, 6, 10.0f) };
		coin.animation.update(i * 0.05f); // So the coins don't all turn in step
		coins.push_back(coin);
	}

	// The sound of collecting a coin. Without an audio device the game still runs, just silently.
	auto audio_device = audio::create_device();
	std::unique_ptr<audio::Sound> coin_sound;
	if (audio_device)
		coin_sound = audio_device->load_sound(core::File::app_path("assets/sounds/coin.wav"));

	glm::vec2 player = { 160.0f, 90.0f };
	bool facing_left = false;
	int collected = 0;

	while (window->update()) {
		// How long the last frame took (at most a tenth of a second, so nothing jumps after a pause,
		// like while the window is being dragged)
		float seconds = std::min(window->get_frame_time(), 0.1f);

		if (Input::is_key_pressed(Key::Escape))
			window->close();

		// Walk with the arrow keys or WASD
		glm::vec2 direction = { 0.0f, 0.0f };
		if (Input::is_key_down(Key::Left) || Input::is_key_down(Key::A)) direction.x -= 1.0f;
		if (Input::is_key_down(Key::Right) || Input::is_key_down(Key::D)) direction.x += 1.0f;
		if (Input::is_key_down(Key::Up) || Input::is_key_down(Key::W)) direction.y -= 1.0f;
		if (Input::is_key_down(Key::Down) || Input::is_key_down(Key::S)) direction.y += 1.0f;
		if (direction.x != 0.0f || direction.y != 0.0f)
			player += glm::normalize(direction) * 60.0f * seconds; // Game pixels per second

		// Face the way we last walked (the sprite looks right, so flip it to look left)
		if (direction.x < 0.0f)
			facing_left = true;
		else if (direction.x > 0.0f)
			facing_left = false;

		for (Coin& coin : coins) {
			coin.animation.update(seconds);

			if (!coin.collected && glm::distance(coin.center, player) < 8.0f) {
				coin.collected = true;
				collected++;
				if (coin_sound)
					coin_sound->play();
			}
		}

		// Draw the game into the game screen, everything at its own size in pixels
		renderer->begin(*game_screen);

		for (int y = 0; y < rows; ++y) {
			for (int x = 0; x < columns; ++x) {
				glm::vec2 position = { x * tile_size, y * tile_size };
				renderer->draw_sprite(position, { tile_size, tile_size }, tiles.get(tile_at(x, y, columns, rows)));
			}
		}

		// The coins and the player are placed by their center
		for (const Coin& coin : coins) {
			if (!coin.collected)
				renderer->draw_sprite(coin.center, { 10.0f, 10.0f }, coin.animation.get_sprite(), { .origin = { 0.5f, 0.5f } });
		}

		renderer->draw_sprite(player, { 8.0f, 8.0f }, tiles.get(Player), { .origin = { 0.5f, 0.5f }, .flip_x = facing_left });

		// Text on top, each with a dark copy just behind it as a shadow. The built-in font's
		// lines are 10 pixels tall, so at 10 every letter pixel is one game pixel.
		const gfx::Color shadow = { 0.0f, 0.0f, 0.0f, 0.6f };
		const gfx::Color white = { 1.0f, 1.0f, 1.0f, 1.0f };

		std::string score = std::format("Coins: {} of {}", collected, coins.size());
		renderer->draw_text({ 5.0f, 5.0f }, score, 10.0f, shadow);
		renderer->draw_text({ 4.0f, 4.0f }, score, 10.0f, white);

		if (collected == int(coins.size())) {
			const gfx::DrawOptions centered = { .origin = { 0.5f, 0.5f } };
			renderer->draw_text({ 162.0f, 82.0f }, "You found them all!", 20.0f, shadow, centered);
			renderer->draw_text({ 160.0f, 80.0f }, "You found them all!", 20.0f, white, centered);
		}

		renderer->end();

		// Show the game screen in the window, as big as whole-number scaling allows, with black
		// bars around it if the window's shape doesn't match
		gfx::clear({ 0.0f, 0.0f, 0.0f, 1.0f });
		auto [window_width, window_height] = window->get_size();
		gfx::PixelPerfectFit fit = gfx::fit_pixel_perfect(game_size, { float(window_width), float(window_height) });

		renderer->begin();
		renderer->draw_quad(fit.position, fit.size, game_screen->get_texture());
		renderer->end();

		gfx::present(*window);
	}

	return 0;
}
