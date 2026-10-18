#include "modulus/gfx/buffer.hpp"
#include "modulus/core/log.hpp"

#include <glad/glad.h>

namespace modulus::gfx {

	// --- Vertex Buffer Implementation ---

	VertexBuffer::VertexBuffer(const void* data, size_t size) : m_size(size) {
		// 1. Generate a buffer ID
		glGenBuffers(1, &m_renderer_id);

		// 2. Select it (Bind) so future operations affect this buffer
		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);

		// 3. Upload the data to the GPU
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	VertexBuffer::VertexBuffer(size_t size) : m_size(size) {
		glGenBuffers(1, &m_renderer_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);

		// No data yet (nullptr). GL_DYNAMIC_DRAW tells the driver that the data will change often.
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::set_data(const void* data, size_t size) {
		if (size > m_size) {
			MOD_ERROR("VertexBuffer::set_data: {} bytes don't fit in a buffer of {} bytes", size, m_size);
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
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

		// Upload through GL_ARRAY_BUFFER, not GL_ELEMENT_ARRAY_BUFFER: the element array binding is part of
		// whichever VAO is bound right now, so binding it here would replace that VAO's index buffer.
		// (A buffer doesn't remember how it was filled, so it still works as an index buffer later.)
		glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);

		// Calculate size: count * sizeof(unsigned int)
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
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