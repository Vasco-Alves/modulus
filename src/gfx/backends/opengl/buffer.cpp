#include <modulus/gfx/buffer.hpp>

#include <glad/glad.h>

namespace modulus::gfx {

	// --- Vertex Buffer Implementation ---

	VertexBuffer::VertexBuffer(const void* data, size_t size) {
		// 1. Generate a buffer ID
		glGenBuffers(1, &m_renderer_id);

		// 2. Select it (Bind) so future operations affect this buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);

		// 3. Upload the data to the GPU
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_renderer_id);
	}

	void VertexBuffer::bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
	}

	void VertexBuffer::unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// --- Index Buffer Implementation ---

	IndexBuffer::IndexBuffer(const unsigned int* indices, unsigned int count) : m_count(count) {
		glGenBuffers(1, &m_renderer_id);

		// Note: We use GL_ELEMENT_ARRAY_BUFFER for indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);

		// Calculate size: count * sizeof(unsigned int)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer() {
		glDeleteBuffers(1, &m_renderer_id);
	}

	void IndexBuffer::bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
	}

	void IndexBuffer::unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

} // namespace modulus::gfx