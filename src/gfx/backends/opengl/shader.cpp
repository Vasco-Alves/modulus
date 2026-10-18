#include "modulus/gfx/shader.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace modulus::gfx {

	// Helper: Compiles one stage (vertex or fragment).
	// Returns 0 if it doesn't compile, after logging the compiler's error message.
	static unsigned int compile_shader(unsigned int type, const std::string& source) {
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();

		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[1024];
			glGetShaderInfoLog(id, 1024, nullptr, info_log);
			MOD_ERROR("{} shader compilation error:\n{}", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", info_log);

			glDeleteShader(id);
			return 0;
		}
		return id;
	}

	Shader::Shader(const std::string& vertex_src, const std::string& fragment_src) {
		unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
		unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);

		// Only link if both stages compiled. Otherwise m_renderer_id stays 0, which means "not valid".
		if (vs != 0 && fs != 0) {
			m_renderer_id = glCreateProgram();
			glAttachShader(m_renderer_id, vs);
			glAttachShader(m_renderer_id, fs);
			glLinkProgram(m_renderer_id);

			int success = 0;
			glGetProgramiv(m_renderer_id, GL_LINK_STATUS, &success);
			if (!success) {
				char info_log[1024];
				glGetProgramInfoLog(m_renderer_id, 1024, nullptr, info_log);
				MOD_ERROR("Shader linking error:\n{}", info_log);

				glDeleteProgram(m_renderer_id);
				m_renderer_id = 0;
			}
		}

		// The program keeps what it needs, so the compiled stages can go.
		// (A stage that failed is 0 here, and deleting 0 does nothing.)
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	std::shared_ptr<Shader> Shader::create(const std::string& vertex_src, const std::string& fragment_src) {
		auto shader = std::make_shared<Shader>(vertex_src, fragment_src);
		if (!shader->is_valid())
			return nullptr; // The error was already logged

		return shader;
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
	// glUniform* changes the uniforms of whichever shader is bound,
	// so each setter binds this shader first.

	void Shader::set_int(const std::string& name, int value) {
		int loc = get_cached_uniform_location(name);
		if (loc == -1) return;
		bind();
		glUniform1i(loc, value);
	}

	void Shader::set_float3(const std::string& name, float x, float y, float z) {
		int loc = get_cached_uniform_location(name);
		if (loc == -1) return;
		bind();
		glUniform3f(loc, x, y, z);
	}

	void Shader::set_float4(const std::string& name, float x, float y, float z, float w) {
		int loc = get_cached_uniform_location(name);
		if (loc == -1) return;
		bind();
		glUniform4f(loc, x, y, z, w);
	}

	void Shader::set_mat4(const std::string& name, const glm::mat4& value) {
		int loc = get_cached_uniform_location(name);
		if (loc == -1) return;
		bind();
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
	}

	int Shader::get_cached_uniform_location(const std::string& name) const {
		// A shader that didn't compile has no uniforms (and its error was already logged)
		if (!is_valid())
			return -1;

		// 1. Check Cache
		auto it = m_uniform_location_cache.find(name);
		if (it != m_uniform_location_cache.end())
			return it->second;

		// 2. Not found? Look it up in OpenGL
		int location = glGetUniformLocation(m_renderer_id, name.c_str());

		if (location == -1) {
			MOD_WARN("Uniform '{0}' does not exist (or was optimized away)!", name);
		}

		// 3. Add to Cache
		m_uniform_location_cache[name] = location;

		return location;
	}

} // namespace modulus::gfx