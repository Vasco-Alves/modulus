// Draws with the 2D renderer into an offscreen image and checks the pixels (see opengl_test.hpp)

#include "opengl_test.hpp"

#include <modulus/gfx/renderer_2d.hpp>
#include <modulus/gfx/texture.hpp>

#include <cstdint>
#include <memory>
#include <vector>

using namespace modulus;

namespace {

	const gfx::Color red = { 1.0f, 0.0f, 0.0f, 1.0f };
	const gfx::Color green = { 0.0f, 1.0f, 0.0f, 1.0f };
	const gfx::Color blue = { 0.0f, 0.0f, 1.0f, 1.0f };

	std::shared_ptr<gfx::Texture2D> make_texture(uint8_t r, uint8_t g, uint8_t b) {
		const uint8_t pixel[] = { r, g, b, 255 };
		return gfx::Texture2D::create(1, 1, pixel);
	}

	// Each test draws into an offscreen image the size of the window (so world units are its
	// pixels), cleared to blue
	class Renderer2DTest : public OpenGLTest {
	protected:
		void SetUp() override {
			OpenGLTest::SetUp();
			if (IsSkipped())
				return;

			target = std::make_unique<OffscreenTarget>(window_width, window_height);
			ASSERT_TRUE(target->is_complete());

			renderer = gfx::Renderer2D::create(*window);
			ASSERT_NE(renderer, nullptr);

			gfx::clear(blue);
		}

		// Destroyed in reverse order, and before the window (which belongs to the base class)
		std::unique_ptr<OffscreenTarget> target;
		std::unique_ptr<gfx::Renderer2D> renderer;
	};

} // namespace

TEST_F(Renderer2DTest, DrawsAQuadWhereAsked) {
	renderer->begin();
	renderer->draw_quad({ 10.0f, 20.0f }, { 30.0f, 40.0f }, red); // x from 10 to 40, y from 20 to 60
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(10, 20), 255, 0, 0)); // Its top-left pixel
	EXPECT_TRUE(has_color(target->pixel(39, 59), 255, 0, 0)); // Its bottom-right pixel
	EXPECT_TRUE(has_color(target->pixel(9, 30), 0, 0, 255));  // Just left of it
	EXPECT_TRUE(has_color(target->pixel(20, 60), 0, 0, 255)); // Just below it
}

TEST_F(Renderer2DTest, LaterQuadsCoverEarlierOnes) {
	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 50.0f, 50.0f }, red);
	renderer->draw_quad({ 25.0f, 25.0f }, { 50.0f, 50.0f }, green);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(10, 10), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(40, 40), 0, 255, 0)); // Where they overlap, the later one is on top
}

TEST_F(Renderer2DTest, LeavesTheDepthTestAsItWas) {
	ASSERT_TRUE(glIsEnabled(GL_DEPTH_TEST)); // gfx::init() turns it on

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 10.0f, 10.0f }, red);
	renderer->end();

	EXPECT_TRUE(glIsEnabled(GL_DEPTH_TEST));
}

TEST_F(Renderer2DTest, CameraScrollsAndZooms) {
	gfx::Camera2D camera;
	camera.position = { 100.0f, 0.0f };
	camera.zoom = 2.0f;

	renderer->begin(camera);
	renderer->draw_quad({ 100.0f, 0.0f }, { 10.0f, 10.0f }, red); // At the top-left, twice as big
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(15, 15), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(25, 25), 0, 0, 255));
}

TEST_F(Renderer2DTest, TexturesAreTheRightWayUp) {
	// 2x2 pixels, top row first: red, green, then yellow, white
	const uint8_t pixels[] = {
		255, 0, 0, 255,     0, 255, 0, 255,
		255, 255, 0, 255,   255, 255, 255, 255,
	};
	auto texture = gfx::Texture2D::create(2, 2, pixels);
	ASSERT_NE(texture, nullptr);

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 64.0f, 64.0f }, *texture);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(16, 16), 255, 0, 0));     // Top-left: red
	EXPECT_TRUE(has_color(target->pixel(48, 16), 0, 255, 0));     // Top-right: green
	EXPECT_TRUE(has_color(target->pixel(16, 48), 255, 255, 0));   // Bottom-left: yellow
	EXPECT_TRUE(has_color(target->pixel(48, 48), 255, 255, 255)); // Bottom-right: white
}

TEST_F(Renderer2DTest, TintMultipliesTheTextureColors) {
	auto white = make_texture(255, 255, 255);
	ASSERT_NE(white, nullptr);

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 20.0f, 20.0f }, *white, { .tint = green });
	renderer->draw_quad({ 30.0f, 0.0f }, { 20.0f, 20.0f }, gfx::Color{ 1.0f, 1.0f, 0.0f, 1.0f }, { .tint = red }); // Yellow * red
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(10, 10), 0, 255, 0));
	EXPECT_TRUE(has_color(target->pixel(40, 10), 255, 0, 0));
}

TEST_F(Renderer2DTest, OriginIsThePointThatGoesToThePosition) {
	// Origin at the middle of the bottom edge: a 20x40 quad stands on (100, 100),
	// so it covers x 90..110 and y 60..100
	renderer->begin();
	renderer->draw_quad({ 100.0f, 100.0f }, { 20.0f, 40.0f }, red, { .origin = { 0.5f, 1.0f } });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(90, 60), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(109, 99), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(100, 100), 0, 0, 255)); // Just below
	EXPECT_TRUE(has_color(target->pixel(89, 80), 0, 0, 255));   // Just left
}

TEST_F(Renderer2DTest, RotatesAroundTheTopLeftCornerByDefault) {
	// A wide bar from (100, 100) turned a quarter around that corner hangs down to its left:
	// x 80..100, y 100..200
	renderer->begin();
	renderer->draw_quad({ 100.0f, 100.0f }, { 100.0f, 20.0f }, red, { .rotation = 90.0f });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(90, 150), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(150, 110), 0, 0, 255)); // Where the bar was before turning
}

TEST_F(Renderer2DTest, RotatesAroundTheOrigin) {
	// A wide bar centered on (150, 110), turned a quarter around its center, becomes a tall bar:
	// x 140..160, y 60..160
	renderer->begin();
	renderer->draw_quad({ 150.0f, 110.0f }, { 100.0f, 20.0f }, red, { .origin = { 0.5f, 0.5f }, .rotation = 90.0f });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(150, 65), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(150, 155), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(110, 110), 0, 0, 255)); // Where the bar's left end was
}

TEST_F(Renderer2DTest, PositiveAnglesTurnClockwise) {
	// Left half red, right half green
	const uint8_t pixels[] = { 255, 0, 0, 255,   0, 255, 0, 255 };
	auto texture = gfx::Texture2D::create(2, 1, pixels);
	ASSERT_NE(texture, nullptr);

	// A quarter turn clockwise moves the left half to the top
	renderer->begin();
	renderer->draw_quad({ 150.0f, 150.0f }, { 100.0f, 100.0f }, *texture, { .origin = { 0.5f, 0.5f }, .rotation = 90.0f });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(150, 115), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(150, 185), 0, 255, 0));
}

TEST_F(Renderer2DTest, FlipMirrorsTheTexture) {
	const uint8_t left_right[] = { 255, 0, 0, 255,   0, 255, 0, 255 }; // Red | green
	const uint8_t top_bottom[] = { 255, 0, 0, 255,   0, 255, 0, 255 }; // Red over green
	auto horizontal = gfx::Texture2D::create(2, 1, left_right);
	auto vertical = gfx::Texture2D::create(1, 2, top_bottom);
	ASSERT_NE(horizontal, nullptr);
	ASSERT_NE(vertical, nullptr);
	horizontal->set_filter(gfx::TextureFilter::Nearest);
	vertical->set_filter(gfx::TextureFilter::Nearest);

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 64.0f, 32.0f }, *horizontal, { .flip_x = true });
	renderer->draw_quad({ 100.0f, 0.0f }, { 32.0f, 64.0f }, *vertical, { .flip_y = true });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(16, 16), 0, 255, 0, 0));  // Green is on the left now
	EXPECT_TRUE(has_color(target->pixel(48, 16), 255, 0, 0, 0));  // and red on the right
	EXPECT_TRUE(has_color(target->pixel(116, 16), 0, 255, 0, 0)); // Green is on top now
	EXPECT_TRUE(has_color(target->pixel(116, 48), 255, 0, 0, 0)); // and red below
}

TEST_F(Renderer2DTest, ColorsAndTexturesShareOneDrawCall) {
	auto texture = make_texture(255, 255, 255);
	ASSERT_NE(texture, nullptr);

	renderer->begin();
	for (int i = 0; i < 1000; ++i) {
		renderer->draw_quad({ float(i % 300), 0.0f }, { 1.0f, 1.0f }, red);
		renderer->draw_quad({ float(i % 300), 10.0f }, { 1.0f, 1.0f }, *texture);
	}
	renderer->end();

	EXPECT_EQ(renderer->get_stats().quads, 2000u);
	EXPECT_EQ(renderer->get_stats().draw_calls, 1u);
}

TEST_F(Renderer2DTest, AFullBatchStartsANewDrawCall) {
	renderer->begin();
	for (unsigned int i = 0; i < gfx::Renderer2D::max_quads_per_draw_call; ++i)
		renderer->draw_quad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, red);
	renderer->draw_quad({ 100.0f, 100.0f }, { 10.0f, 10.0f }, green); // Goes into the second batch
	renderer->end();

	EXPECT_EQ(renderer->get_stats().quads, gfx::Renderer2D::max_quads_per_draw_call + 1);
	EXPECT_EQ(renderer->get_stats().draw_calls, 2u);
	EXPECT_TRUE(has_color(target->pixel(105, 105), 0, 255, 0)); // The second batch was drawn too
}

TEST_F(Renderer2DTest, RunningOutOfTextureSlotsStartsANewDrawCall) {
	// One texture more than fits in a draw call (slot 0 is the white texture for plain colors).
	// The last one is green, the others white.
	std::vector<std::shared_ptr<gfx::Texture2D>> textures;
	for (int i = 0; i < gfx::Renderer2D::max_textures_per_draw_call - 1; ++i)
		textures.push_back(make_texture(255, 255, 255));
	textures.push_back(make_texture(0, 255, 0));

	renderer->begin();
	for (const auto& texture : textures)
		renderer->draw_quad({ 50.0f, 50.0f }, { 10.0f, 10.0f }, *texture);
	renderer->end();

	EXPECT_EQ(renderer->get_stats().draw_calls, 2u);
	EXPECT_TRUE(has_color(target->pixel(55, 55), 0, 255, 0)); // The last texture was drawn, last
}

TEST_F(Renderer2DTest, StatsAreForTheLastBeginAndEnd) {
	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, red);
	renderer->draw_quad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, red);
	renderer->end();

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, red);
	renderer->end();

	EXPECT_EQ(renderer->get_stats().quads, 1u);
	EXPECT_EQ(renderer->get_stats().draw_calls, 1u);
}

TEST_F(Renderer2DTest, NearestFilterKeepsPixelsSharp) {
	// Red and green side by side, drawn 64 wide: with Nearest, the pixels just left and right
	// of the middle are pure red and pure green (Linear would blend them into each other)
	const uint8_t pixels[] = { 255, 0, 0, 255,   0, 255, 0, 255 };
	auto texture = gfx::Texture2D::create(2, 1, pixels);
	ASSERT_NE(texture, nullptr);
	texture->set_filter(gfx::TextureFilter::Nearest);

	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 64.0f, 8.0f }, *texture);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(31, 4), 255, 0, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(32, 4), 0, 255, 0, 0));
}

TEST_F(Renderer2DTest, DrawsTheRightCellOfASpriteSheet) {
	// A 4x4 texture made of four 2x2 cells, top row first: red, green, then yellow, white
	const uint8_t R[] = { 255, 0, 0, 255 }, G[] = { 0, 255, 0, 255 }, Y[] = { 255, 255, 0, 255 }, W[] = { 255, 255, 255, 255 };
	const uint8_t* rows[4][4] = {
		{ R, R, G, G },
		{ R, R, G, G },
		{ Y, Y, W, W },
		{ Y, Y, W, W },
	};
	std::vector<uint8_t> pixels;
	for (auto& row : rows)
		for (const uint8_t* pixel : row)
			pixels.insert(pixels.end(), pixel, pixel + 4);

	auto texture = gfx::Texture2D::create(4, 4, pixels.data());
	ASSERT_NE(texture, nullptr);
	texture->set_filter(gfx::TextureFilter::Nearest); // So the cells' edges don't blend with their neighbors
	gfx::SpriteSheet sheet(texture, 2, 2);

	renderer->begin();
	renderer->draw_sprite({ 0.0f, 0.0f }, { 32.0f, 32.0f }, sheet.get(1, 0));  // Top-right cell: green
	renderer->draw_sprite({ 40.0f, 0.0f }, { 32.0f, 32.0f }, sheet.get(0, 1)); // Bottom-left cell: yellow
	renderer->end();

	for (int y : { 1, 16, 30 }) {
		for (int x : { 1, 16, 30 }) {
			EXPECT_TRUE(has_color(target->pixel(x, y), 0, 255, 0, 0)) << "at " << x << ", " << y;
			EXPECT_TRUE(has_color(target->pixel(40 + x, y), 255, 255, 0, 0)) << "at " << 40 + x << ", " << y;
		}
	}
}

TEST_F(Renderer2DTest, EmptySpritesDrawNothing) {
	renderer->begin();
	renderer->draw_sprite({ 0.0f, 0.0f }, { 32.0f, 32.0f }, gfx::Sprite{});
	renderer->end();

	EXPECT_EQ(renderer->get_stats().quads, 0u);
	EXPECT_TRUE(has_color(target->pixel(16, 16), 0, 0, 255));
}

TEST_F(Renderer2DTest, FlippedSpritesStayInTheirCell) {
	// A 4x1 texture in cells of 2x1: red, green | white, yellow
	const uint8_t pixels[] = { 255, 0, 0, 255,   0, 255, 0, 255,   255, 255, 255, 255,   255, 255, 0, 255 };
	auto texture = gfx::Texture2D::create(4, 1, pixels);
	ASSERT_NE(texture, nullptr);
	texture->set_filter(gfx::TextureFilter::Nearest);
	gfx::SpriteSheet sheet(texture, 2, 1);

	renderer->begin();
	renderer->draw_sprite({ 0.0f, 0.0f }, { 64.0f, 32.0f }, sheet.get(1), { .flip_x = true });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(16, 16), 255, 255, 0, 0));   // Yellow on the left now
	EXPECT_TRUE(has_color(target->pixel(48, 16), 255, 255, 255, 0)); // and white on the right
}

// The built-in font's 'I' is 5x8 pixels in a 6x10 cell: a bar across the top (columns 1-3), a stem
// down the middle (column 2, rows 1-5), and a bar across the bottom (row 6). At height 20, each of
// its pixels is 2x2.

TEST_F(Renderer2DTest, DrawsTextWithTheBuiltinFont) {
	renderer->begin();
	renderer->draw_text({ 0.0f, 0.0f }, "I", 20.0f, red);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(3, 1), 255, 0, 0, 0));  // The top bar
	EXPECT_TRUE(has_color(target->pixel(4, 6), 255, 0, 0, 0));  // The stem
	EXPECT_TRUE(has_color(target->pixel(0, 6), 0, 0, 255, 0));  // Left of the stem: empty
	EXPECT_TRUE(has_color(target->pixel(8, 6), 0, 0, 255, 0));  // Right of the stem: empty
}

TEST_F(Renderer2DTest, TextOriginIsMeasuredOnTheWholeText) {
	// "II" is 24x20, so centered on (100, 100) it starts at (88, 90)
	renderer->begin();
	renderer->draw_text({ 100.0f, 100.0f }, "II", 20.0f, red, { .origin = { 0.5f, 0.5f } });
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(92, 96), 255, 0, 0, 0));  // The first stem
	EXPECT_TRUE(has_color(target->pixel(104, 96), 255, 0, 0, 0)); // The second stem
	EXPECT_TRUE(has_color(target->pixel(98, 96), 0, 0, 255, 0));  // Between them
}

TEST_F(Renderer2DTest, NewlineStartsANewLine) {
	renderer->begin();
	renderer->draw_text({ 0.0f, 0.0f }, "I\nI", 20.0f, red);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(4, 6), 255, 0, 0, 0));  // First line's stem
	EXPECT_TRUE(has_color(target->pixel(4, 17), 0, 0, 255, 0)); // The gap below it
	EXPECT_TRUE(has_color(target->pixel(4, 26), 255, 0, 0, 0)); // Second line's stem
}

TEST_F(Renderer2DTest, TextBatchesWithEverythingElse) {
	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 10.0f, 10.0f }, red);
	renderer->draw_text({ 0.0f, 20.0f }, "Hi there", 20.0f, green); // 7 characters to draw (not the space)
	renderer->draw_quad({ 0.0f, 50.0f }, { 10.0f, 10.0f }, red);
	renderer->end();

	EXPECT_EQ(renderer->get_stats().quads, 9u);
	EXPECT_EQ(renderer->get_stats().draw_calls, 1u);
}
