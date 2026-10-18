#pragma once

#include "buffer.hpp"
#include "buffer_layout.hpp"

#include <memory>

namespace modulus::gfx {

	class VertexArray {
	public:
		VertexArray();
		~VertexArray();

		// Owns a GPU object, so copying is not allowed (both copies would delete it)
		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;

		void bind() const;
		void unbind() const;

		// The VAO takes ownership of the buffers (via shared_ptr) because the VAO needs them to stay alive to draw.
		// Attribute locations continue across buffers: if the first buffer's layout has 2 attributes
		// (locations 0 and 1), the second buffer's attributes start at location 2.
		void add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vertexBuffer, const BufferLayout& layout);

		void set_index_buffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

		const std::shared_ptr<IndexBuffer>& get_index_buffer() const {
			return m_indexBuffer;
		}

	private:
		unsigned int m_renderer_id;
		unsigned int m_attribute_count = 0; // How many attributes the buffers added so far use
		std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
		std::shared_ptr<IndexBuffer> m_indexBuffer;
	};

} // namespace modulus::gfx