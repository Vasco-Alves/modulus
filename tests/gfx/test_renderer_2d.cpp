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
	renderer->draw_quad({ 0.0f, 0.0f }, { 20.0f, 20.0f }, *white, green);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(10, 10), 0, 255, 0));
}

TEST_F(Renderer2DTest, RotatesAroundTheQuadsCenter) {
	// A wide bar (x 100..200, y 100..120) turned a quarter around its center (150, 110)
	// becomes a tall bar: x 140..160, y 60..160
	renderer->begin();
	renderer->draw_rotated_quad({ 100.0f, 100.0f }, { 100.0f, 20.0f }, 90.0f, red);
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
	renderer->draw_rotated_quad({ 100.0f, 100.0f }, { 100.0f, 100.0f }, 90.0f, *texture);
	renderer->end();

	EXPECT_TRUE(has_color(target->pixel(150, 115), 255, 0, 0));
	EXPECT_TRUE(has_color(target->pixel(150, 185), 0, 255, 0));
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
