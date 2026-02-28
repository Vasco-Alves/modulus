#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

namespace modulus::gfx {

	class Shader {
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void bind() const;
		void unbind() const;

		void set_int(const std::string& name, int value);
		void set_float3(const std::string& name, float x, float y, float z);
		void set_float4(const std::string& name, float x, float y, float z, float w);
		void set_mat4(const std::string& name, const glm::mat4& value);

		// Utility: Returns the raw ID
		unsigned int get_id() const {
			return m_renderer_id;
		}

	private:
		unsigned int m_renderer_id = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;

		int get_cached_uniform_location(const std::string& name) const;
	};

} // namespace modulus::gfx