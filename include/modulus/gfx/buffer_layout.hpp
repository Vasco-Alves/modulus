#pragma once

#include <vector>
#include <string>

namespace modulus::gfx {

	enum class ShaderDataType {
		None = 0, Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, Bool
	};

	// Helper to get size in bytes of a type
	static unsigned int shader_data_type_size(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}
		return 0;
	}

	struct BufferElement {
		std::string name;
		ShaderDataType type;
		unsigned int size;
		unsigned int offset;
		bool normalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), type(type), size(shader_data_type_size(type)), offset(0), normalized(normalized) {
		}

		// Helper: How many components? (Float3 = 3)
		unsigned int get_component_count() const {
			switch (type) {
			case ShaderDataType::Float:   return 1;
			case ShaderDataType::Float2:  return 2;
			case ShaderDataType::Float3:  return 3;
			case ShaderDataType::Float4:  return 4;
			case ShaderDataType::Int:     return 1;
			case ShaderDataType::Int2:    return 2;
			case ShaderDataType::Int3:    return 3;
			case ShaderDataType::Int4:    return 4;
			case ShaderDataType::Bool:    return 1;
			}
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_elements(elements) {
			calculate_offsets_and_stride();
		}

		inline unsigned int get_stride() const {
			return m_stride;
		}

		inline const std::vector<BufferElement>& get_elements() const {
			return m_elements;
		}

	private:
		void calculate_offsets_and_stride() {
			unsigned int offset = 0;
			m_stride = 0;
			for (auto& element : m_elements) {
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		std::vector<BufferElement> m_elements;
		unsigned int m_stride = 0;
	};

} // namespace modulus::gfx