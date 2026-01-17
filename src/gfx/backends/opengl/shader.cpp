#include <modulus/gfx/shader.hpp>
#include <modulus/core/log.hpp>
#include <glad/glad.h>

#include <vector>

namespace modulus::gfx {

	// Helper: Checks if compilation/linking failed and logs the error
	// 'id' is the shader or program ID.
	// 'isProgram' is true if checking linking, false if checking compilation.
	static void check_compile_errors(unsigned int id, bool isProgram) {
		int success;
		char infoLog[1024];

		if (isProgram) {
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(id, 1024, nullptr, infoLog);
				MOD_ERROR("Shader Linking Error:\n{0}", infoLog);
			}
		} else {
			glGetShaderiv(id, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(id, 1024, nullptr, infoLog);
				MOD_ERROR("Shader Compilation Error:\n{0}", infoLog);
			}
		}
	}

	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		// 1. Create the Program Container
		m_renderer_id = glCreateProgram();

		// 2. Compile the individual shaders
		unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSrc);
		unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSrc);

		// 3. Attach and Link
		glAttachShader(m_renderer_id, vs);
		glAttachShader(m_renderer_id, fs);
		glLinkProgram(m_renderer_id);

		// 4. Check for Linking Errors
		check_compile_errors(m_renderer_id, true);

		// 5. Cleanup
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	Shader::~Shader() {
		glDeleteProgram(m_renderer_id);
	}

	void Shader::bind() const {
		glUseProgram(m_renderer_id);
	}

	void Shader::unbind() const {
		glUseProgram(0);
	}

	unsigned int Shader::compile_shader(unsigned int type, const std::string& source) {
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();

		// Load source and compile
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		// Check for errors
		check_compile_errors(id, false);

		return id;
	}

	void Shader::set_int(const std::string& name, int value) {
		// Note: In a real engine, we would cache this location to avoid string lookup!
		int loc = glGetUniformLocation(m_renderer_id, name.c_str());
		if (loc != -1) glUniform1i(loc, value);
	}

	void Shader::set_float3(const std::string& name, float x, float y, float z) {
		int loc = glGetUniformLocation(m_renderer_id, name.c_str());
		if (loc != -1) glUniform3f(loc, x, y, z);
	}

	void Shader::set_mat4(const std::string& name, const float* value) {
		int loc = glGetUniformLocation(m_renderer_id, name.c_str());
		// 1 = count, GL_FALSE = transpose? (usually false for GLM)
		if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, value);
	}

} // namespace modulus::gfx