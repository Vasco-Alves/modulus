// Sprite sheets and animations. They only do arithmetic, but they need real textures (see opengl_test.hpp).

#include "opengl_test.hpp"

#include <modulus/gfx/sprite_sheet.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace modulus;

namespace {

	class SpriteSheetTest : public OpenGLTest {};

	// A texture of the given size. Its colors don't matter for these tests.
	std::shared_ptr<gfx::Texture2D> blank_texture(int width, int height) {
		std::vector<uint8_t> pixels(size_t(width) * height * 4, 255);
		return gfx::Texture2D::create(width, height, pixels.data());
	}

	void expect_region(const gfx::Sprite& sprite, float x, float y, float width, float height) {
		EXPECT_NE(sprite.texture, nullptr);
		EXPECT_FLOAT_EQ(sprite.region.x, x);
		EXPECT_FLOAT_EQ(sprite.region.y, y);
		EXPECT_FLOAT_EQ(sprite.region.width, width);
		EXPECT_FLOAT_EQ(sprite.region.height, height);
	}

} // namespace

TEST_F(SpriteSheetTest, CountsItsCells) {
	gfx::SpriteSheet sheet(blank_texture(64, 32), 16, 16);
	EXPECT_EQ(sheet.get_columns(), 4);
	EXPECT_EQ(sheet.get_rows(), 2);
	EXPECT_EQ(sheet.get_count(), 8);
}

TEST_F(SpriteSheetTest, IgnoresPartlyCutOffCells) {
	gfx::SpriteSheet sheet(blank_texture(40, 20), 16, 16); // Room for 2.5 x 1.25 cells
	EXPECT_EQ(sheet.get_columns(), 2);
	EXPECT_EQ(sheet.get_rows(), 1);
}

TEST_F(SpriteSheetTest, CellsByColumnAndRow) {
	gfx::SpriteSheet sheet(blank_texture(64, 32), 16, 16);
	expect_region(sheet.get(0, 0), 0.0f, 0.0f, 16.0f, 16.0f);
	expect_region(sheet.get(2, 1), 32.0f, 16.0f, 16.0f, 16.0f);
}

TEST_F(SpriteSheetTest, CellsByIndexGoLeftToRightThenDown) {
	gfx::SpriteSheet sheet(blank_texture(64, 32), 16, 16); // 4 columns
	expect_region(sheet.get(3), 48.0f, 0.0f, 16.0f, 16.0f);
	expect_region(sheet.get(5), 16.0f, 16.0f, 16.0f, 16.0f); // Second row
}

TEST_F(SpriteSheetTest, SpacingSkipsTheGapsBetweenCells) {
	// 3 cells of 16 pixels with 1 empty pixel between them: 16 + 1 + 16 + 1 + 16 = 50
	gfx::SpriteSheet sheet(blank_texture(50, 16), 16, 16, 1);
	EXPECT_EQ(sheet.get_columns(), 3);
	expect_region(sheet.get(1, 0), 17.0f, 0.0f, 16.0f, 16.0f);
	expect_region(sheet.get(2, 0), 34.0f, 0.0f, 16.0f, 16.0f);
}

TEST_F(SpriteSheetTest, MissingCellsAreEmptyAndLogged) {
	gfx::SpriteSheet sheet(blank_texture(32, 16), 16, 16);

	testing::internal::CaptureStderr();
	gfx::Sprite by_position = sheet.get(2, 0);
	gfx::Sprite by_index = sheet.get(-1);
	std::string errors = testing::internal::GetCapturedStderr();

	EXPECT_EQ(by_position.texture, nullptr);
	EXPECT_EQ(by_index.texture, nullptr);
	EXPECT_NE(errors.find("no cell"), std::string::npos) << errors;
}

TEST_F(SpriteSheetTest, SheetWithoutATextureHasNoCells) {
	gfx::SpriteSheet sheet(nullptr, 16, 16);
	EXPECT_EQ(sheet.get_count(), 0);
	EXPECT_EQ(sheet.get(0).texture, nullptr);
}

TEST_F(SpriteSheetTest, AnimationMovesThroughItsFrames) {
	gfx::SpriteSheet sheet(blank_texture(64, 16), 16, 16);
	gfx::SpriteAnimation animation(sheet, 0, 4, 10.0f); // 4 frames, 0.1 seconds each

	EXPECT_EQ(animation.get_frame(), 0);
	animation.update(0.15f);
	EXPECT_EQ(animation.get_frame(), 1);
	animation.update(0.2f); // 0.35 seconds in
	EXPECT_EQ(animation.get_frame(), 3);
	animation.update(0.1f); // 0.45: past the end, so it started over
	EXPECT_EQ(animation.get_frame(), 0);
	EXPECT_FALSE(animation.is_finished());
}

TEST_F(SpriteSheetTest, AnimationShowsTheRightCells) {
	gfx::SpriteSheet sheet(blank_texture(64, 32), 16, 16);
	gfx::SpriteAnimation animation(sheet, 4, 2, 10.0f); // Cells 4 and 5: the second row

	expect_region(animation.get_sprite(), 0.0f, 16.0f, 16.0f, 16.0f);
	animation.update(0.15f);
	expect_region(animation.get_sprite(), 16.0f, 16.0f, 16.0f, 16.0f);
}

TEST_F(SpriteSheetTest, AnimationWithoutLoopingStopsOnItsLastFrame) {
	gfx::SpriteSheet sheet(blank_texture(48, 16), 16, 16);
	gfx::SpriteAnimation animation(sheet, 0, 3, 10.0f, false);

	animation.update(0.25f);
	EXPECT_EQ(animation.get_frame(), 2);
	EXPECT_FALSE(animation.is_finished()); // The last frame still has 0.05 seconds to go

	animation.update(1.0f);
	EXPECT_EQ(animation.get_frame(), 2);
	EXPECT_TRUE(animation.is_finished());

	animation.restart();
	EXPECT_EQ(animation.get_frame(), 0);
	EXPECT_FALSE(animation.is_finished());
}
