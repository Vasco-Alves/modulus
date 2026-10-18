#include <modulus/gfx/vertex_array.hpp>

#include <glad/glad.h>

namespace modulus::gfx {

	// Helper to convert our enum to OpenGL enum
	static GLenum shader_data_type_to_opengl_base_type(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::None:     return 0;
		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Float2:   return GL_FLOAT;
		case ShaderDataType::Float3:   return GL_FLOAT;
		case ShaderDataType::Float4:   return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Int2:     return GL_INT;
		case ShaderDataType::Int3:     return GL_INT;
		case ShaderDataType::Int4:     return GL_INT;
		}
		return 0;
	}

	VertexArray::VertexArray() {
		glGenVertexArrays(1, &m_renderer_id);
	}

	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_renderer_id);
	}

	void VertexArray::bind() const {
		glBindVertexArray(m_renderer_id);
	}

	void VertexArray::unbind() const {
		glBindVertexArray(0);
	}

	void VertexArray::add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, const BufferLayout& layout) {
		// 1. Bind VAO
		glBindVertexArray(m_renderer_id);

		// 2. Bind VBO
		vertexBuffer->bind();

		// 3. Set Attributes (The Recipe)
		// Attributes are numbered across all the vertex buffers, so the first attribute
		// of a second buffer continues where the first buffer stopped.
		for (const auto& element : layout.get_elements()) {
			glEnableVertexAttribArray(m_attribute_count);

			GLenum base_type = shader_data_type_to_opengl_base_type(element.type);
			const void* offset = (const void*)(uintptr_t)element.offset; // Cast offset to pointer

			// This tells OpenGL:
			// "Attribute 'index' is composed of 'count' 'type's."
			// "It is 'normalized'?"
			// "The whole vertex is 'stride' bytes big."
			// "This attribute starts 'offset' bytes into the vertex."
			if (base_type == GL_INT) {
				// Integer inputs (int, ivec2, ...) need the "I" version: glVertexAttribPointer
				// would turn the values into floats, and the shader would read garbage.
				glVertexAttribIPointer(m_attribute_count, element.get_component_count(), base_type, layout.get_stride(), offset);
			} else {
				glVertexAttribPointer(
					m_attribute_count,
					element.get_component_count(),
					base_type,
					element.normalized ? GL_TRUE : GL_FALSE,
					layout.get_stride(),
					offset
				);
			}

			m_attribute_count++;
		}

		// Unbind, so that binding buffers later can't change this VAO by accident
		glBindVertexArray(0);

		m_vertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::set_index_buffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_renderer_id);
		indexBuffer->bind(); // Binding IBO while VAO is active ties them together

		// Unbind the VAO (but not the IBO: unbinding that now would untie them again)
		glBindVertexArray(0);

		m_indexBuffer = indexBuffer;
	}

} // namespace modulus::gfx