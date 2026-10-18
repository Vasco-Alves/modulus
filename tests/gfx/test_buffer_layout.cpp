#include <gtest/gtest.h>
#include <modulus/gfx/buffer_layout.hpp>

using modulus::gfx::BufferLayout;
using modulus::gfx::ShaderDataType;

TEST(BufferLayout, OffsetsFollowEachOther) {
	BufferLayout layout = {
		{ ShaderDataType::Float3, "a_Position" }, // 12 bytes
		{ ShaderDataType::Float4, "a_Color" },    // 16 bytes
		{ ShaderDataType::Float2, "a_TexCoord" }, //  8 bytes
		{ ShaderDataType::Int, "a_EntityID" },    //  4 bytes
	};

	const auto& elements = layout.get_elements();
	ASSERT_EQ(elements.size(), 4u);
	EXPECT_EQ(elements[0].offset, 0u);
	EXPECT_EQ(elements[1].offset, 12u);
	EXPECT_EQ(elements[2].offset, 28u);
	EXPECT_EQ(elements[3].offset, 36u);
	EXPECT_EQ(layout.get_stride(), 40u);
}

TEST(BufferLayout, ComponentCounts) {
	BufferLayout layout = {
		{ ShaderDataType::Float, "a_Float" },
		{ ShaderDataType::Float3, "a_Float3" },
		{ ShaderDataType::Int2, "a_Int2" },
		{ ShaderDataType::Int4, "a_Int4" },
	};

	const auto& elements = layout.get_elements();
	ASSERT_EQ(elements.size(), 4u);
	EXPECT_EQ(elements[0].get_component_count(), 1u);
	EXPECT_EQ(elements[1].get_component_count(), 3u);
	EXPECT_EQ(elements[2].get_component_count(), 2u);
	EXPECT_EQ(elements[3].get_component_count(), 4u);
}

TEST(BufferLayout, EmptyLayoutHasNoStride) {
	BufferLayout layout;
	EXPECT_TRUE(layout.get_elements().empty());
	EXPECT_EQ(layout.get_stride(), 0u);
}
