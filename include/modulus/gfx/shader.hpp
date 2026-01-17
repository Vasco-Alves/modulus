#pragma once

#include <string>

using ShaderID = unsigned int;

namespace modulus::gfx {

	class Shader {
	public:
		// Constructor: Takes the source code for vertex and fragment shaders
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);

		// Destructor: Cleans up the shader program on GPU
		~Shader();

		// Activates this shader (calls glUseProgram)
		void bind() const;

		void unbind() const;

		void set_int(const std::string& name, int value);

		void set_float3(const std::string& name, float x, float y, float z);

		void set_mat4(const std::string& name, const float* value); // Pointer to float array (for matrices)

		// Utility: Returns the raw OpenGL ID
		ShaderID get_id() const {
			return m_renderer_id;
		}

	private:
		ShaderID m_renderer_id = 0;

		ShaderID compile_shader(unsigned int type, const std::string& source);
	};

} // namespace modulus::gfx