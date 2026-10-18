#include <gtest/gtest.h>
#include <modulus/gfx/camera_2d.hpp>

using modulus::gfx::Camera2D;

namespace {

	// Where a world point ends up in OpenGL's -1..1 range
	glm::vec2 to_opengl(const Camera2D& camera, glm::vec2 window_size, glm::vec2 world_point) {
		glm::vec4 clip = camera.get_view_projection(window_size) * glm::vec4(world_point, 0.0f, 1.0f);
		return { clip.x / clip.w, clip.y / clip.w };
	}

	void expect_near(glm::vec2 actual, glm::vec2 expected) {
		EXPECT_NEAR(actual.x, expected.x, 1e-5f);
		EXPECT_NEAR(actual.y, expected.y, 1e-5f);
	}

} // namespace

TEST(Camera2D, DefaultCameraShowsWindowPixelsWithYDown) {
	Camera2D camera;
	glm::vec2 window = { 800.0f, 600.0f };

	expect_near(to_opengl(camera, window, { 0.0f, 0.0f }), { -1.0f, 1.0f });     // Top-left
	expect_near(to_opengl(camera, window, { 800.0f, 600.0f }), { 1.0f, -1.0f }); // Bottom-right
	expect_near(to_opengl(camera, window, { 400.0f, 300.0f }), { 0.0f, 0.0f });  // Center

	expect_near(camera.screen_to_world({ 10.0f, 20.0f }), { 10.0f, 20.0f });
}

TEST(Camera2D, PositionScrollsTheView) {
	Camera2D camera;
	camera.position = { 100.0f, 50.0f };
	glm::vec2 window = { 800.0f, 600.0f };

	expect_near(to_opengl(camera, window, { 100.0f, 50.0f }), { -1.0f, 1.0f }); // Now at the top-left
	expect_near(camera.screen_to_world({ 0.0f, 0.0f }), { 100.0f, 50.0f });
}

TEST(Camera2D, ZoomMakesThingsBigger) {
	Camera2D camera;
	camera.zoom = 2.0f;
	glm::vec2 window = { 800.0f, 600.0f };

	// Half the world fits now: (400, 300) reaches the bottom-right corner
	expect_near(to_opengl(camera, window, { 400.0f, 300.0f }), { 1.0f, -1.0f });
	expect_near(camera.world_to_screen({ 10.0f, 10.0f }), { 20.0f, 20.0f });
}

TEST(Camera2D, ScreenAndWorldConversionsUndoEachOther) {
	Camera2D camera;
	camera.position = { -30.0f, 75.0f };
	camera.zoom = 1.5f;

	glm::vec2 point = { 123.0f, 45.0f };
	expect_near(camera.world_to_screen(camera.screen_to_world(point)), point);
}

TEST(Camera2D, ZoomAtKeepsThePointUnderTheMouse) {
	Camera2D camera;
	camera.position = { 10.0f, 20.0f };
	glm::vec2 mouse = { 300.0f, 200.0f };

	glm::vec2 before = camera.screen_to_world(mouse);
	camera.zoom_at(mouse, 3.0f);

	EXPECT_FLOAT_EQ(camera.zoom, 3.0f);
	expect_near(camera.screen_to_world(mouse), before);
}

TEST(Camera2D, ZoomAtIgnoresZoomsOfZeroOrLess) {
	Camera2D camera;
	camera.zoom_at({ 10.0f, 10.0f }, 0.0f);
	EXPECT_FLOAT_EQ(camera.zoom, 1.0f);
}
