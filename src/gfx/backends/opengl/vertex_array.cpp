#include <modulus/gfx/vertex_array.hpp>

#include <glad/glad.h>

namespace modulus::gfx {

	// Helper to convert our enum to OpenGL enum
	static GLenum shader_data_type_to_opengl_base_type(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Float2:   return GL_FLOAT;
		case ShaderDataType::Float3:   return GL_FLOAT;
		case ShaderDataType::Float4:   return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Int2:     return GL_INT;
		case ShaderDataType::Int3:     return GL_INT;
		case ShaderDataType::Int4:     return GL_INT;
		case ShaderDataType::Bool:     return GL_BOOL;
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
		const auto& elements = layout.get_elements();
		unsigned int index = 0;

		for (const auto& element : elements) {
			glEnableVertexAttribArray(index);

			// This tells OpenGL:
			// "Attribute 'index' is composed of 'count' 'type's."
			// "It is 'normalized'?"
			// "The whole vertex is 'stride' bytes big."
			// "This attribute starts 'offset' bytes into the vertex."
			glVertexAttribPointer(
				index,
				element.get_component_count(),
				shader_data_type_to_opengl_base_type(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.get_stride(),
				(const void*)(uintptr_t)element.offset // Cast offset to pointer
			);

			index++;
		}

		m_vertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::set_index_buffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_renderer_id);
		indexBuffer->bind(); // Binding IBO while VAO is active ties them together
		m_indexBuffer = indexBuffer;
	}

} // namespace modulus::gfx