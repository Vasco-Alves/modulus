// These tests need a real OpenGL 3.3 context, so each one opens a hidden window.
// Where that isn't possible (no GPU or no display, like on many CI servers) they skip themselves.

#include <glad/glad.h> // Before GLFW, so GLFW doesn't include the system's OpenGL header
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/shader.hpp>
#include <modulus/gfx/vertex_array.hpp>
#include <modulus/platform/platform.hpp>

#include <memory>

using namespace modulus;

namespace {

	class OpenGLTest : public ::testing::Test {
	protected:
		void SetUp() override {
			platform::WindowConfig config;
			config.title = "modulus_tests";
			config.width = 320;
			config.height = 240;
			config.visible = false;

			window = platform::create_window(config);
			if (!window || !gfx::init(*window))
				GTEST_SKIP() << "No OpenGL 3.3 context available";
		}

		std::unique_ptr<platform::IWindow> window;

		// Data for the buffers. The tests only check how buffers are set up, not what is in them.
		float vertices[64] = {};
		unsigned int indices[6] = { 0, 1, 2, 2, 3, 0 };
	};

	GLint get_integer(GLenum name) {
		GLint value = 0;
		glGetIntegerv(name, &value);
		return value;
	}

	// Reads a property of vertex attribute 'index' of the bound vertex array
	GLint get_attribute(GLuint index, GLenum name) {
		GLint value = 0;
		glGetVertexAttribiv(index, name, &value);
		return value;
	}

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
	const int size = 64;
	GLuint framebuffer = 0, color_buffer = 0;
	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &color_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size, size);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer);
	ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GLenum(GL_FRAMEBUFFER_COMPLETE));
	gfx::set_viewport(0, 0, size, size);

	gfx::Shader shader(
		R"(#version 330 core
		layout(location = 0) in vec3 a_Position;
		void main() { gl_Position = vec4(a_Position, 1.0); })",

		R"(#version 330 core
		layout(location = 0) out vec4 o_Color;
		void main() { o_Color = vec4(1.0, 0.0, 0.0, 1.0); })"
	);
	GLint linked = GL_FALSE;
	glGetProgramiv(shader.get_id(), GL_LINK_STATUS, &linked);
	ASSERT_EQ(linked, GL_TRUE);

	// A triangle around the center of the image
	float triangle[] = { -0.5f, -0.5f, 0.0f,   0.5f, -0.5f, 0.0f,   0.0f, 0.5f, 0.0f };
	unsigned int triangle_indices[] = { 0, 1, 2 };

	auto mesh = std::make_shared<gfx::VertexArray>();
	mesh->add_vertex_buffer(std::make_shared<gfx::VertexBuffer>(triangle, sizeof(triangle)),
		{ { gfx::ShaderDataType::Float3, "a_Position" } });
	mesh->set_index_buffer(std::make_shared<gfx::IndexBuffer>(triangle_indices, 3));

	gfx::clear({ 0.0f, 0.0f, 1.0f, 1.0f }); // Blue
	shader.bind();
	gfx::draw_indexed(mesh);

	unsigned char center[4] = {}, corner[4] = {};
	glReadPixels(size / 2, size / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, center);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, corner);

	EXPECT_EQ(int(center[0]), 255); // The triangle is red...
	EXPECT_EQ(int(center[2]), 0);
	EXPECT_EQ(int(corner[0]), 0);   // ...and the corner keeps the blue background
	EXPECT_EQ(int(corner[2]), 255);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &color_buffer);
	glDeleteFramebuffers(1, &framebuffer);
}
