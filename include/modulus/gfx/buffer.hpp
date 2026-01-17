#pragma once

#include <vector>

namespace modulus::gfx {

	// =========================================================================
	// Vertex Buffer: Raw data storage (Positions, Colors, etc.)
	// =========================================================================
	class VertexBuffer {
	public:
		// Create buffer and upload data immediately
		// size: Size in bytes
		VertexBuffer(const void* data, size_t size);
		~VertexBuffer();

		void bind() const;
		void unbind() const;

	private:
		unsigned int m_renderer_id;
	};

	// =========================================================================
	// Index Buffer: Defines connection order
	// =========================================================================
	class IndexBuffer {
	public:
		// count: The number of INDICES (integers), not bytes
		IndexBuffer(const unsigned int* indices, unsigned int count);
		~IndexBuffer();

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