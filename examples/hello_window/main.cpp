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
	// 1. Create Window (this also starts the platform module)
	modulus::platform::WindowConfig config;
	config.title = "Modulus Engine - First Triangle";
	config.width = 800;
	config.height = 600;

	auto window = modulus::platform::create_window(config);
	if (!window)
		return -1;

	// 2. Start Graphics (it draws into the window, so it needs one)
	if (!modulus::gfx::init(*window))
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

	// app_path() finds the assets next to the program, whatever folder it was started from
	auto vert_src = modulus::core::File::read_text(modulus::core::File::app_path("assets/shaders/triangle.vert"));
	auto frag_src = modulus::core::File::read_text(modulus::core::File::app_path("assets/shaders/triangle.frag"));

	if (!vert_src || !frag_src) {
		MOD_FATAL("Could not load shaders! Is the assets folder next to the program?");
		return -1;
	}

	// create() returns nullptr if the shader doesn't compile (the compiler's error is logged)
	auto shader = modulus::gfx::Shader::create(*vert_src, *frag_src);
	if (!shader)
		return -1;

	// =========================================================================
	// 5. Start Audio
	// =========================================================================

	// Audio plays while 'audio' exists, like the platform runs while the window exists
	auto audio = modulus::audio::create_device();
	if (!audio)
		return -1;

	std::string sound_path = modulus::core::File::app_path("assets/test.mp3");

	// =========================================================================
	// 6. Main Loop
	// =========================================================================
	while (window->update()) {
		if (modulus::platform::Input::is_key_down(modulus::platform::Key::Escape)) {
			window->close();
		}

		// "pressed", not "down": play once per press, not every frame while held
		if (modulus::platform::Input::is_key_pressed(modulus::platform::Key::Space)) {
			audio->play_sound(sound_path);
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

	// 7. Cleanup
	// Nothing to do: audio, the graphics objects, the window and the platform
	// are all cleaned up automatically when main() ends.
	return 0;
}
