// Render targets and pixel-perfect scaling. The scaling math needs no GPU; drawing into a render
// target needs a real OpenGL context (see opengl_test.hpp).

#include "opengl_test.hpp"

#include <modulus/gfx/render_target.hpp>
#include <modulus/gfx/renderer_2d.hpp>

#include <cstdint>
#include <memory>
#include <vector>

using namespace modulus;

namespace {

	const gfx::Color red = { 1.0f, 0.0f, 0.0f, 1.0f };
	const gfx::Color blue = { 0.0f, 0.0f, 1.0f, 1.0f };

	// Each test has a renderer, and an offscreen image the size of the window as "the screen"
	class RenderTargetTest : public OpenGLTest {
	protected:
		void SetUp() override {
			OpenGLTest::SetUp();
			if (IsSkipped())
				return;

			screen = std::make_unique<OffscreenTarget>(window_width, window_height);
			ASSERT_TRUE(screen->is_complete());

			renderer = gfx::Renderer2D::create(*window);
			ASSERT_NE(renderer, nullptr);
		}

		std::unique_ptr<OffscreenTarget> screen;
		std::unique_ptr<gfx::Renderer2D> renderer;
	};

	// The pixels of a texture: result[y][x], with x and y from its top-left corner
	std::vector<std::vector<std::array<int, 4>>> read_texture(const gfx::Texture2D& texture) {
		int width = texture.get_width();
		int height = texture.get_height();
		std::vector<unsigned char> rgba(size_t(width) * height * 4);
		texture.bind();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

		std::vector<std::vector<std::array<int, 4>>> result(height, std::vector<std::array<int, 4>>(width));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				const unsigned char* p = &rgba[(size_t(height - 1 - y) * width + x) * 4]; // OpenGL's rows go up
				result[y][x] = { p[0], p[1], p[2], p[3] };
			}
		}
		return result;
	}

	void expect_vec2(glm::vec2 actual, float x, float y) {
		EXPECT_FLOAT_EQ(actual.x, x);
		EXPECT_FLOAT_EQ(actual.y, y);
	}

} // namespace

TEST(PixelPerfectFit, ScalesByWholeNumbers) {
	// 320x180 fits into 1280x720 exactly 4 times
	gfx::PixelPerfectFit exact = gfx::fit_pixel_perfect({ 320.0f, 180.0f }, { 1280.0f, 720.0f });
	EXPECT_FLOAT_EQ(exact.scale, 4.0f);
	expect_vec2(exact.position, 0.0f, 0.0f);
	expect_vec2(exact.size, 1280.0f, 720.0f);

	// Into 1000x600 it would fit 3.33 times: so 3 times, centered, with bars around it
	gfx::PixelPerfectFit bars = gfx::fit_pixel_perfect({ 320.0f, 180.0f }, { 1000.0f, 600.0f });
	EXPECT_FLOAT_EQ(bars.scale, 3.0f);
	expect_vec2(bars.size, 960.0f, 540.0f);
	expect_vec2(bars.position, 20.0f, 30.0f);
}

TEST(PixelPerfectFit, ShrinksWhenTheWindowIsTooSmall) {
	gfx::PixelPerfectFit fit = gfx::fit_pixel_perfect({ 320.0f, 180.0f }, { 160.0f, 160.0f });
	EXPECT_FLOAT_EQ(fit.scale, 0.5f);
	expect_vec2(fit.size, 160.0f, 90.0f);
	expect_vec2(fit.position, 0.0f, 35.0f);
}

TEST(PixelPerfectFit, TurnsWindowPointsIntoImagePoints) {
	gfx::PixelPerfectFit fit = gfx::fit_pixel_perfect({ 320.0f, 180.0f }, { 1000.0f, 600.0f }); // 3 times, at (20, 30)
	expect_vec2(fit.window_to_image({ 20.0f + 3 * 10.0f, 30.0f + 3 * 5.0f }), 10.0f, 5.0f);
}

TEST_F(RenderTargetTest, CreateRejectsBadSizes) {
	EXPECT_EQ(gfx::RenderTarget::create(0, 10), nullptr);
	EXPECT_EQ(gfx::RenderTarget::create(10, -1), nullptr);
}

TEST_F(RenderTargetTest, DrawsIntoTheTarget) {
	auto target = gfx::RenderTarget::create(32, 18);
	ASSERT_NE(target, nullptr);
	target->clear(blue);

	renderer->begin(*target);
	renderer->draw_quad({ 0.0f, 0.0f }, { 16.0f, 9.0f }, red); // The top-left quarter
	renderer->draw_quad({ 24.0f, 0.0f }, { 8.0f, 9.0f }, red); // At the right edge, if the target's size is used
	renderer->end();

	auto pixels = read_texture(target->get_texture());
	EXPECT_TRUE(has_color(pixels[0][0], 255, 0, 0, 0));
	EXPECT_TRUE(has_color(pixels[8][15], 255, 0, 0, 0));  // The quarter's last pixel
	EXPECT_TRUE(has_color(pixels[9][0], 0, 0, 255, 0));   // Just below it
	EXPECT_TRUE(has_color(pixels[0][16], 0, 0, 255, 0));  // Just right of it
	EXPECT_TRUE(has_color(pixels[4][31], 255, 0, 0, 0));  // The second quad, at the right edge
	EXPECT_TRUE(has_color(pixels[17][31], 0, 0, 255, 0)); // The bottom-right corner
}

TEST_F(RenderTargetTest, EndGoesBackToWhereItWasDrawing) {
	GLint framebuffer_before = get_integer(GL_FRAMEBUFFER_BINDING);
	GLint viewport_before[4] = {};
	glGetIntegerv(GL_VIEWPORT, viewport_before);

	auto target = gfx::RenderTarget::create(32, 18);
	ASSERT_NE(target, nullptr);
	renderer->begin(*target);
	renderer->draw_quad({ 0.0f, 0.0f }, { 16.0f, 9.0f }, red);
	renderer->end();

	EXPECT_EQ(get_integer(GL_FRAMEBUFFER_BINDING), framebuffer_before);
	GLint viewport_after[4] = {};
	glGetIntegerv(GL_VIEWPORT, viewport_after);
	for (int i = 0; i < 4; ++i)
		EXPECT_EQ(viewport_after[i], viewport_before[i]);
}

TEST_F(RenderTargetTest, ItsTextureIsTheRightWayUp) {
	auto target = gfx::RenderTarget::create(32, 18);
	ASSERT_NE(target, nullptr);
	target->clear(blue);

	renderer->begin(*target);
	renderer->draw_quad({ 0.0f, 0.0f }, { 16.0f, 9.0f }, red); // The top-left quarter
	renderer->end();

	// Draw it twice as big on the screen: every target pixel becomes 2x2
	gfx::clear({ 0.0f, 0.0f, 0.0f, 1.0f });
	renderer->begin();
	renderer->draw_quad({ 0.0f, 0.0f }, { 64.0f, 36.0f }, target->get_texture());
	renderer->end();

	EXPECT_TRUE(has_color(screen->pixel(0, 0), 255, 0, 0, 0));   // Red at the top-left...
	EXPECT_TRUE(has_color(screen->pixel(31, 17), 255, 0, 0, 0)); // ...until target pixel (15, 8)
	EXPECT_TRUE(has_color(screen->pixel(32, 17), 0, 0, 255, 0)); // Then blue
	EXPECT_TRUE(has_color(screen->pixel(63, 35), 0, 0, 255, 0));
	EXPECT_TRUE(has_color(screen->pixel(64, 36), 0, 0, 0, 0));   // Outside it, the screen's black
}
