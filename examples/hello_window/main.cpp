#include <modulus/core/context.hpp>
#include <modulus/core/log.hpp>
#include <modulus/core/file.hpp>

#include <modulus/platform/platform.hpp>
#include <modulus/platform/input.hpp>

#include <modulus/gfx/graphics.hpp>
#include <modulus/gfx/shader.hpp>
#include <modulus/gfx/buffer.hpp>
#include <modulus/gfx/vertex_array.hpp>

#include <modulus/audio/audio.hpp>

int main() {
	// 1. Initialize System
	modulus::Context ctx;
	if (!ctx.is_valid())
		return -1;

	if (!modulus::audio::init())
		return -1;

	// 2. Create Window
	modulus::platform::WindowConfig config;
	config.title = "Modulus Engine - First Triangle";
	config.width = 800;
	config.height = 600;

	auto window = modulus::platform::create_window(config);
	if (!window)
		return -1;

	if (!modulus::gfx::init())
		return -1;

	// =========================================================================
	// 3. Setup Geometry (The "Mesh")
	// =========================================================================

	// X, Y, Z,   R, G, B
	float vertices[] = {
		 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // Top (Red)
		-0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // Bottom Left (Green)
		 0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f  // Bottom Right (Blue)
	};

	unsigned int indices[] = { 0, 1, 2 };

	// Create Vertex Array (The Container)
	auto vao = std::make_shared<modulus::gfx::VertexArray>();

	// Create Vertex Buffer (The Data)
	auto vbo = std::make_shared<modulus::gfx::VertexBuffer>(vertices, sizeof(vertices));

	// Create Layout (The Description: "3 floats pos, 3 floats color")
	modulus::gfx::BufferLayout layout = {
		{ modulus::gfx::ShaderDataType::Float3, "a_Position" },
		{ modulus::gfx::ShaderDataType::Float3, "a_Color" }
	};

	// Link VBO + Layout to VAO
	vao->add_vertex_buffer(vbo, layout);

	// Create Index Buffer (The Order)
	auto ibo = std::make_shared<modulus::gfx::IndexBuffer>(indices, 3);
	vao->set_index_buffer(ibo);

	// =========================================================================
	// 4. Setup Shader (The "Painter")
	// =========================================================================

	auto vertSrc = modulus::core::File::read_text("assets/shaders/triangle.vert");
	auto fragSrc = modulus::core::File::read_text("assets/shaders/triangle.frag");

	if (!vertSrc || !fragSrc) {
		MOD_FATAL("Could not load shaders! Check your working directory.");
		return -1;
	}

	auto shader = std::make_shared<modulus::gfx::Shader>(*vertSrc, *fragSrc);

	// =========================================================================
	// 5. Main Loop
	// =========================================================================
	while (window->update()) {
		if (modulus::platform::Input::is_key_down(modulus::platform::Key::Escape)) {
			window->close();
		}

		if (modulus::platform::Input::is_key_down(modulus::platform::Key::Space)) {
			modulus::audio::play_sound("assets/test.mp3");
		}

		// 1. Clear Screen
		modulus::gfx::clear({ 0.1f, 0.1f, 0.1f, 1.0f }); // Dark Grey

		// 2. Bind Resources
		shader->bind();
		vao->bind(); // Binding VAO automatically binds VBO and IBO

		// 3. Draw! 
		modulus::gfx::draw_indexed(vao);

		// 4. Present
		modulus::gfx::present(*window);
	}

	// 6. Cleanup
	modulus::audio::shutdown();

	return 0;
}


/*
#include <modulus/core/log.hpp>
#include <modulus/platform/platform.hpp>

int main() {
	MOD_INFO("Starting Hello Window Example...");

	// 1. Initialize Platform (GLFW)
	if (!modulus::platform::init()) {
		return -1;
	}

	// 2. Create a Window
	modulus::platform::WindowConfig config;
	config.title = "Hello Modulus!";
	config.width = 800;
	config.height = 600;

	auto window = modulus::platform::create_window(config);
	if (!window) {
		modulus::platform::shutdown();
		return -1;
	}

	// 3. Main Loop
	while (window->update()) {
		// In the future:
		// gfx::clear();
		// game::update();
		// gfx::present();
	}

	// 4. Cleanup
	window.reset(); // Destroy window before shutting down GLFW
	modulus::platform::shutdown();

	return 0;
}
*/

/*
#include <modulus/core/context.hpp>
#include <modulus/platform/platform.hpp>
#include <modulus/platform/input.hpp>
#include <modulus/gfx/graphics.hpp>

int main() {
	modulus::Context ctx;
	if (!ctx.is_valid()) return -1;

	modulus::platform::WindowConfig config;
	config.title = "Modulus Graphics";
	auto window = modulus::platform::create_window(config);
	if (!window) return -1;

	if (!modulus::gfx::init()) return -1;

	while (window->update()) {
		// Update
		{
			// Input
			if (modulus::platform::Input::is_key_down(modulus::platform::Key::Escape)) {
				window->close();
			}
		}

		// Draw
		{
			modulus::gfx::clear({ 0.2f, 0.4f, 0.8f, 1.0f });
			modulus::gfx::present(*window);
		}
	}

	return 0;
}
*/