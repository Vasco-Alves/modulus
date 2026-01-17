#pragma once

#include "buffer.hpp"
#include "buffer_layout.hpp"

#include <memory>

namespace modulus::gfx {

	class VertexArray {
	public:
		VertexArray();
		~VertexArray();

		void bind() const;
		void unbind() const;

		// The VAO takes ownership of the buffers (via shared_ptr) because the VAO needs them to stay alive to draw.
		void add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, const BufferLayout& layout);
		void set_index_buffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

		const std::shared_ptr<IndexBuffer>& get_index_buffer() const {
			return m_indexBuffer;
		}

	private:
		unsigned int m_renderer_id;
		std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
		std::shared_ptr<IndexBuffer> m_indexBuffer;
	};

} // namespace modulus::gfx