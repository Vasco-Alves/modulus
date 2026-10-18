// Tests of the graphics module's building blocks, against a real OpenGL context (see opengl_test.hpp)

#include "opengl_test.hpp"

#include <modulus/gfx/shader.hpp>
#include <modulus/gfx/texture.hpp>
#include <modulus/gfx/vertex_array.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace modulus;

namespace {

	// Passes the position through
	const char* position_vertex_shader = R"(#version 330 core
		layout(location = 0) in vec3 a_Position;
		void main() { gl_Position = vec4(a_Position, 1.0); })";

	// Paints everything red
	const char* red_fragment_shader = R"(#version 330 core
		layout(location = 0) out vec4 o_Color;
		void main() { o_Color = vec4(1.0, 0.0, 0.0, 1.0); })";

} // namespace

TEST_F(OpenGLTest, EachVertexBufferGetsItsOwnAttributes) {
	// Positions in one buffer, colors in another
	auto positions = std::make_shared<gfx::VertexBuffer>(vertices, sizeof(vertices));
	auto colors = std::make_shared<gfx::VertexBuffer>(vertices, sizeof(vertices));

	gfx::VertexArray vertex_array;
	vertex_array.add_vertex_buffer(positions, { { gfx::ShaderDataType::Float3, "a_Position" } });
	vertex_array.add_vertex_buffer(colors, { { gfx::ShaderDataType::Float4, "a_Color" } });
	vertex_array.bind();

	// The colors continue at location 1, instead of replacing the positions at location 0
	EXPECT_EQ(get_attribute(0, GL_VERTEX_ATTRIB_ARRAY_SIZE), 3);
	EXPECT_EQ(get_attribute(1, GL_VERTEX_ATTRIB_ARRAY_SIZE), 4);
	EXPECT_EQ(get_attribute(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED), GL_TRUE);
	EXPECT_NE(get_attribute(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING),
	          get_attribute(1, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING));
}

TEST_F(OpenGLTest, IntegerAttributesStayIntegers) {
	auto buffer = std::make_shared<gfx::VertexBuffer>(vertices, sizeof(vertices));

	gfx::VertexArray vertex_array;
	vertex_array.add_vertex_buffer(buffer, {
		{ gfx::ShaderDataType::Int, "a_EntityID" },
		{ gfx::ShaderDataType::Float2, "a_TexCoord" },
	});
	vertex_array.bind();

	EXPECT_EQ(get_attribute(0, GL_VERTEX_ATTRIB_ARRAY_INTEGER), GL_TRUE);
	EXPECT_EQ(get_attribute(1, GL_VERTEX_ATTRIB_ARRAY_INTEGER), GL_FALSE);
}

TEST_F(OpenGLTest, EveryLayoutTypeIsAccepted) {
	while (glGetError() != GL_NO_ERROR) {} // Only look at errors from setting up the layout

	auto buffer = std::make_shared<gfx::VertexBuffer>(vertices, sizeof(vertices));

	gfx::VertexArray vertex_array;
	vertex_array.add_vertex_buffer(buffer, {
		{ gfx::ShaderDataType::Float, "a_Float" },
		{ gfx::ShaderDataType::Float2, "a_Float2" },
		{ gfx::ShaderDataType::Float3, "a_Float3" },
		{ gfx::ShaderDataType::Float4, "a_Float4" },
		{ gfx::ShaderDataType::Int, "a_Int" },
		{ gfx::ShaderDataType::Int2, "a_Int2" },
		{ gfx::ShaderDataType::Int3, "a_Int3" },
		{ gfx::ShaderDataType::Int4, "a_Int4" },
	});

	EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
}

TEST_F(OpenGLTest, CreatingAnIndexBufferLeavesTheBoundVertexArrayAlone) {
	gfx::VertexArray mesh;
	mesh.add_vertex_buffer(std::make_shared<gfx::VertexBuffer>(vertices, sizeof(vertices)),
		{ { gfx::ShaderDataType::Float3, "a_Position" } });
	mesh.set_index_buffer(std::make_shared<gfx::IndexBuffer>(indices, 6));

	mesh.bind();
	GLint mesh_indices = get_integer(GL_ELEMENT_ARRAY_BUFFER_BINDING);
	ASSERT_NE(mesh_indices, 0);

	// Another mesh creates its index buffer while the first one is still bound
	auto other_indices = std::make_shared<gfx::IndexBuffer>(indices, 3);

	EXPECT_EQ(get_integer(GL_ELEMENT_ARRAY_BUFFER_BINDING), mesh_indices);
}

TEST_F(OpenGLTest, ViewportFollowsTheWindowSize) {
	auto [start_width, start_height] = window->get_framebuffer_size();

	glfwSetWindowSize(static_cast<GLFWwindow*>(window->get_native_handle()), 640, 480);
	for (int i = 0; i < 10; ++i) // Some window systems deliver the resize a few events later
		window->update();

	auto [width, height] = window->get_framebuffer_size();
	if (width == start_width && height == start_height)
		GTEST_SKIP() << "This window system didn't resize the hidden window";

	GLint viewport[4] = {};
	glGetIntegerv(GL_VIEWPORT, viewport);
	EXPECT_EQ(viewport[2], width);
	EXPECT_EQ(viewport[3], height);
}

TEST_F(OpenGLTest, DrawsATriangle) {
	// Draw into an offscreen image (a framebuffer object): a hidden window may have no pixels to read back
	OffscreenTarget target(64, 64);
	ASSERT_TRUE(target.is_complete());

	auto shader = gfx::Shader::create(position_vertex_shader, red_fragment_shader);
	ASSERT_NE(shader, nullptr);

	// A triangle around the center of the image
	float triangle[] = { -0.5f, -0.5f, 0.0f,   0.5f, -0.5f, 0.0f,   0.0f, 0.5f, 0.0f };
	unsigned int triangle_indices[] = { 0, 1, 2 };

	auto mesh = std::make_shared<gfx::VertexArray>();
	mesh->add_vertex_buffer(std::make_shared<gfx::VertexBuffer>(triangle, sizeof(triangle)),
		{ { gfx::ShaderDataType::Float3, "a_Position" } });
	mesh->set_index_buffer(std::make_shared<gfx::IndexBuffer>(triangle_indices, 3));

	gfx::clear({ 0.0f, 0.0f, 1.0f, 1.0f }); // Blue
	shader->bind();
	gfx::draw_indexed(mesh);

	EXPECT_TRUE(has_color(target.pixel(32, 32), 255, 0, 0, 0)); // The triangle is red...
	EXPECT_TRUE(has_color(target.pixel(0, 0), 0, 0, 255, 0));   // ...and the corner keeps the blue background
}

TEST_F(OpenGLTest, SettingAUniformChangesThatShader) {
	// Uses u_Value, so the compiler can't optimize it away
	const char* fragment_shader = R"(#version 330 core
		uniform int u_Value;
		layout(location = 0) out vec4 o_Color;
		void main() { o_Color = vec4(float(u_Value)); })";

	auto first = gfx::Shader::create(position_vertex_shader, fragment_shader);
	auto second = gfx::Shader::create(position_vertex_shader, fragment_shader);
	ASSERT_NE(first, nullptr);
	ASSERT_NE(second, nullptr);

	second->bind();
	first->set_int("u_Value", 7); // While the other shader is bound

	GLint value = 0;
	glGetUniformiv(first->get_id(), glGetUniformLocation(first->get_id(), "u_Value"), &value);
	EXPECT_EQ(value, 7);
}

TEST_F(OpenGLTest, ErrorsAreLoggedInDebugBuilds) {
#ifdef NDEBUG
	GTEST_SKIP() << "Only debug builds log OpenGL errors";
#else
	if (!glIsEnabled(GL_DEBUG_OUTPUT)) {
		glGetError(); // Without GL_KHR_debug, even asking is an error
		GTEST_SKIP() << "This driver has no debug output (GL_KHR_debug)";
	}

	testing::internal::CaptureStderr();
	glBindVertexArray(12345); // Never created, so OpenGL refuses it
	std::string errors = testing::internal::GetCapturedStderr();
	glGetError();

	EXPECT_NE(errors.find("OpenGL: "), std::string::npos) << errors;
#endif
}

TEST_F(OpenGLTest, ShaderCreateReturnsNullWhenCompilingFails) {
	EXPECT_EQ(gfx::Shader::create("this is not GLSL", red_fragment_shader), nullptr);
	EXPECT_EQ(gfx::Shader::create(position_vertex_shader, "neither is this"), nullptr);

	gfx::Shader shader("this is not GLSL", red_fragment_shader);
	EXPECT_FALSE(shader.is_valid());
	shader.set_int("u_Anything", 1); // Does nothing, instead of calling OpenGL with a broken shader
}

TEST_F(OpenGLTest, ShaderCreateReturnsNullWhenLinkingFails) {
	// This fragment shader compiles, but it has no main(), so the program can't be linked
	const char* fragment_shader = R"(#version 330 core
		layout(location = 0) out vec4 o_Color;
		vec4 red() { return vec4(1.0, 0.0, 0.0, 1.0); })";

	testing::internal::CaptureStderr();
	auto shader = gfx::Shader::create(position_vertex_shader, fragment_shader);
	std::string errors = testing::internal::GetCapturedStderr();

	EXPECT_EQ(shader, nullptr);
	EXPECT_NE(errors.find("Shader linking error"), std::string::npos) << errors;
}

TEST_F(OpenGLTest, TextureCreateReturnsNullForAMissingFile) {
	EXPECT_EQ(gfx::Texture2D::create("this/image/does/not/exist.png"), nullptr);
}

TEST_F(OpenGLTest, TextureLoadsAnImage) {
	// A 2x1 image in the PPM format, which is simple enough to write by hand: a red and a blue pixel
	std::filesystem::path path = std::filesystem::temp_directory_path() / "modulus_test_texture.ppm";
	{
		std::ofstream out(path, std::ios::binary);
		out << "P6\n2 1\n255\n";
		const unsigned char pixels[] = { 255, 0, 0,   0, 0, 255 };
		out.write(reinterpret_cast<const char*>(pixels), sizeof(pixels));
	}

	auto texture = gfx::Texture2D::create(path.string());
	std::filesystem::remove(path);

	ASSERT_NE(texture, nullptr);
	EXPECT_EQ(texture->get_width(), 2);
	EXPECT_EQ(texture->get_height(), 1);
	EXPECT_NE(texture->get_renderer_id(), 0u);
}

TEST_F(OpenGLTest, VertexBufferSetDataReplacesTheContents) {
	gfx::VertexBuffer buffer(4 * sizeof(float));
	const float data[] = { 1.0f, 2.0f, 3.0f, 4.0f };
	buffer.set_data(data, sizeof(data));

	float stored[4] = {};
	buffer.bind();
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(stored), stored);
	for (int i = 0; i < 4; ++i)
		EXPECT_EQ(stored[i], data[i]);
}

TEST_F(OpenGLTest, VertexBufferSetDataRefusesTooMuchData) {
	while (glGetError() != GL_NO_ERROR) {}
	gfx::VertexBuffer buffer(8);
	const float data[4] = {};

	testing::internal::CaptureStderr();
	buffer.set_data(data, sizeof(data)); // 16 bytes into 8
	std::string errors = testing::internal::GetCapturedStderr();

	EXPECT_NE(errors.find("don't fit"), std::string::npos) << errors;
	EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR)); // OpenGL wasn't even asked
}

TEST_F(OpenGLTest, TexturesFromFilesAndFromPixelsAreStoredTheSameWay) {
	// A 1x2 image: a red pixel on top of a blue one
	std::filesystem::path path = std::filesystem::temp_directory_path() / "modulus_test_orientation.ppm";
	{
		std::ofstream out(path, std::ios::binary);
		out << "P6\n1 2\n255\n";
		const unsigned char rgb[] = { 255, 0, 0,   0, 0, 255 };
		out.write(reinterpret_cast<const char*>(rgb), sizeof(rgb));
	}
	auto from_file = gfx::Texture2D::create(path.string());
	std::filesystem::remove(path);

	const uint8_t rgba[] = { 255, 0, 0, 255,   0, 0, 255, 255 }; // The same, top row first
	auto from_pixels = gfx::Texture2D::create(1, 2, rgba);

	ASSERT_NE(from_file, nullptr);
	ASSERT_NE(from_pixels, nullptr);
	EXPECT_EQ(from_pixels->get_width(), 1);
	EXPECT_EQ(from_pixels->get_height(), 2);

	auto stored = [](const gfx::Texture2D& texture) {
		std::vector<uint8_t> pixels(8);
		texture.bind();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
		return pixels;
	};
	EXPECT_EQ(stored(*from_pixels), stored(*from_file));

	// OpenGL keeps the bottom row first: blue, then red
	EXPECT_EQ(stored(*from_pixels), (std::vector<uint8_t>{ 0, 0, 255, 255,   255, 0, 0, 255 }));
}

TEST_F(OpenGLTest, TextureFromPixelsRejectsBadInput) {
	const uint8_t white[] = { 255, 255, 255, 255 };
	EXPECT_EQ(gfx::Texture2D::create(0, 1, white), nullptr);
	EXPECT_EQ(gfx::Texture2D::create(1, 1, nullptr), nullptr);
}

TEST_F(OpenGLTest, SetIntArraySetsEveryElement) {
	// Uses every element, so the compiler can't optimize them away
	const char* fragment_shader = R"(#version 330 core
		uniform int u_Values[3];
		layout(location = 0) out vec4 o_Color;
		void main() { o_Color = vec4(float(u_Values[0] + u_Values[1] + u_Values[2])); })";

	auto shader = gfx::Shader::create(position_vertex_shader, fragment_shader);
	ASSERT_NE(shader, nullptr);

	const int values[] = { 4, 5, 6 };
	shader->set_int_array("u_Values", values, 3);

	for (int i = 0; i < 3; ++i) {
		std::string name = "u_Values[" + std::to_string(i) + "]";
		GLint value = 0;
		glGetUniformiv(shader->get_id(), glGetUniformLocation(shader->get_id(), name.c_str()), &value);
		EXPECT_EQ(value, values[i]) << name;
	}
}

TEST_F(OpenGLTest, DrawIndexedWithoutAnIndexBufferLogsAnError) {
	gfx::VertexArray vertex_array;

	testing::internal::CaptureStderr();
	gfx::draw_indexed(vertex_array); // Used to crash
	std::string errors = testing::internal::GetCapturedStderr();

	EXPECT_NE(errors.find("no index buffer"), std::string::npos) << errors;
}
