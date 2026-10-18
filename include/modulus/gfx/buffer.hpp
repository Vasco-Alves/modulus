#pragma once

#include <cstddef>

namespace modulus::gfx {

	// =========================================================================
	// Vertex Buffer: Raw data storage (Positions, Colors, etc.)
	// =========================================================================
	class VertexBuffer {
	public:
		// Create buffer and upload data immediately
		// size: Size in bytes
		VertexBuffer(const void* data, size_t size);

		// Create an empty buffer of 'size' bytes, to fill (again and again) with set_data().
		// For data that changes every frame, like the vertices of a batch of sprites.
		explicit VertexBuffer(size_t size);
		~VertexBuffer();

		// Owns a GPU object, so copying is not allowed (both copies would delete it)
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;

		void bind() const;
		void unbind() const;

		// Replaces the start of the buffer with 'size' bytes of 'data' (at most the buffer's size)
		void set_data(const void* data, size_t size);

	private:
		unsigned int m_renderer_id;
		size_t m_size; // In bytes
	};

	// =========================================================================
	// Index Buffer: Defines connection order
	// =========================================================================
	class IndexBuffer {
	public:
		// count: The number of INDICES (integers), not bytes
		IndexBuffer(const unsigned int* indices, unsigned int count);
		~IndexBuffer();

		// Owns a GPU object, so copying is not allowed (both copies would delete it)
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;

		// Careful: these change the index buffer of whichever VertexArray is bound at the time
		void bind() const;
		void unbind() const;

		unsigned int get_count() const {
			return m_count;
		}

	private:
		unsigned int m_renderer_id;
		unsigned int m_count;
	};

} // namespace modulus::gfx