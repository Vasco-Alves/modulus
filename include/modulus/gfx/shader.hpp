#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace modulus::gfx {

	class Shader {
	public:
		// Compiles and links the shader. If that fails, the error is logged and is_valid() returns false.
		Shader(const std::string& vertex_src, const std::string& fragment_src);
		~Shader();

		// Like the constructor, but returns nullptr if the shader doesn't compile or link
		// (the error is logged), so you can check it with: if (!shader)
		static std::shared_ptr<Shader> create(const std::string& vertex_src, const std::string& fragment_src);

		// Owns a GPU object, so copying is not allowed (both copies would delete it)
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		// False if the shader didn't compile or link
		bool is_valid() const {
			return m_renderer_id != 0;
		}

		void bind() const;
		void unbind() const;

		// Setting a uniform also binds this shader (OpenGL can only change the bound shader's uniforms)
		void set_int(const std::string& name, int value);
		void set_float3(const std::string& name, float x, float y, float z);
		void set_float4(const std::string& name, float x, float y, float z, float w);
		void set_mat4(const std::string& name, const glm::mat4& value);
		void set_int_array(const std::string& name, const int* values, int count); // For example the slots of a sampler array

		// Utility: Returns the raw ID
		unsigned int get_id() const {
			return m_renderer_id;
		}

	private:
		unsigned int m_renderer_id = 0;
		mutable std::unordered_map<std::string, int> m_uniform_location_cache;

		int get_cached_uniform_location(const std::string& name) const;
	};

} // namespace modulus::gfx