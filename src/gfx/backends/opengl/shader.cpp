#include "modulus/gfx/shader.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace modulus::gfx {

	// Helper: Checks if compilation/linking failed and logs the error
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

	// Helper: Compilation logic
	static unsigned int compile_shader(unsigned int type, const std::string& source) {
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();

		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		check_compile_errors(id, false);
		return id;
	}

	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		m_renderer_id = glCreateProgram();

		unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertexSrc);
		unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragmentSrc);

		glAttachShader(m_renderer_id, vs);
		glAttachShader(m_renderer_id, fs);
		glLinkProgram(m_renderer_id);

		check_compile_errors(m_renderer_id, true);

		// Ideally detach before deleting, but strictly deleting works too
		// as they are flagged for deletion when detached/program destroyed.
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

	// --- Uniforms (Now consistently using the cache) ---

	void Shader::set_int(const std::string& name, int value) {
		int loc = get_cached_uniform_location(name);
		if (loc != -1) glUniform1i(loc, value);
	}

	void Shader::set_float3(const std::string& name, float x, float y, float z) {
		int loc = get_cached_uniform_location(name);
		if (loc != -1) glUniform3f(loc, x, y, z);
	}

	void Shader::set_float4(const std::string& name, float x, float y, float z, float w) {
		int loc = get_cached_uniform_location(name);
		if (loc != -1) glUniform4f(loc, x, y, z, w);
	}

	void Shader::set_mat4(const std::string& name, const glm::mat4& value) {
		int loc = get_cached_uniform_location(name);
		if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
	}

	int Shader::get_cached_uniform_location(const std::string& name) const {
		// 1. Check Cache
		auto it = m_UniformLocationCache.find(name);
		if (it != m_UniformLocationCache.end())
			return it->second;

		// 2. Not found? Look it up in OpenGL
		int location = glGetUniformLocation(m_renderer_id, name.c_str());

		if (location == -1) {
			MOD_WARN("Uniform '{0}' does not exist (or was optimized away)!", name);
		}

		// 3. Add to Cache
		m_UniformLocationCache[name] = location;

		return location;
	}

} // namespace modulus::gfx