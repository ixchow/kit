#pragma once

/*
 * This header declares classes that wrap GL Buffer objects.
 * Specifically:
 * A GLBuffer just wraps a buffer object; everything else is up to you.
 * A GLAttribBuffer< > wraps a vertex attribute buffer, and remembers both
 *  the type of the attributes stored in that buffer and how many vertices are stored.
 *
 */

#include "gl.hpp"

#include "GLTypeInfo.hpp"

#include <glm/glm.hpp>

#include <vector>

//GLBuffer is a thin wrapper around a buffer:
struct GLBuffer {
	GLuint buffer = 0;

	GLBuffer() { glGenBuffers(1, &buffer); }
	~GLBuffer() { if (buffer != 0) glDeleteBuffers(1, &buffer); }
	GLBuffer(GLBuffer const &) = delete;
	GLBuffer(GLBuffer &&from) { std::swap(buffer, from.buffer); }
	GLBuffer &operator=(GLBuffer &&from) { std::swap(buffer, from.buffer); return *this; }

	void set(GLenum target, GLsizeiptr size, GLvoid const *data, GLenum usage) {
		glBindBuffer(target, buffer);
		glBufferData(target, size, data, usage);
	}
};

//AttribPointer identifies a location within a buffer (e.g. when constructing a binding):
struct GLAttribPointer {
	GLuint buffer = 0;

	GLint size = 0;
	GLenum type = 0;
	uint8_t interpretation = 0;
	//GLboolean normalized = GL_FALSE;
	GLsizei stride = 0;
	GLsizei offset = 0; //offset into the buffer, in bytes

	GLAttribPointer() = default;
	GLAttribPointer(GLuint _buffer, GLint _size, GLenum _type, uint8_t _interpretation, GLsizei _stride, GLsizei _offset)
		: buffer(_buffer), size(_size), type(_type), interpretation(_interpretation), stride(_stride), offset(_offset) { }
};

//GLAttribBuffer< A0, A1, ... > is a buffer that stores interleaved vertex attribute data of a specific type, and remembers both this type and how many vertices are stored.
// As a convenience, a compatible vertex type is available as GLAttribBuffer< A0, A1, ... >::Vertex.
template< typename... A >
struct GLAttribBuffer;

template< typename A0 >
struct GLAttribBuffer< A0 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_) : a0(std::forward< A0 >(a0_)) { }
		Vertex() = default;
		A0 a0;
	};
	static_assert(sizeof(Vertex) == sizeof(A0), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		assert(idx == 0);
		return GLAttribPointer(buffer,
			GLTypeInfo< A0 >::size,
			GLTypeInfo< A0 >::type,
			GLTypeInfo< A0 >::interpretation,
			sizeof(Vertex),
			0
		);
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};

template< typename A0, typename A1 >
struct GLAttribBuffer< A0, A1 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_, A1 &&a1_) : a0(std::forward< A0 >(a0_)), a1(std::forward< A1 >(a1_)) { }
		Vertex() = default;
		A0 a0;
		A1 a1;
	};
	static_assert(sizeof(Vertex) == sizeof(A0) + sizeof(A1), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		if (idx == 0) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A0 >::size,
				GLTypeInfo< A0 >::type,
				GLTypeInfo< A0 >::interpretation,
				sizeof(Vertex),
				0
			);
		} else if (idx == 1) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A1 >::size,
				GLTypeInfo< A1 >::type,
				GLTypeInfo< A1 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0)
			);
		} else {
			assert(idx < 2);
			return GLAttribPointer();
		}
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};

template< typename A0, typename A1, typename A2 >
struct GLAttribBuffer< A0, A1, A2 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_, A1 &&a1_, A2 &&a2_) : a0(std::forward(a0_)), a1(std::forward(a1_)), a2(std::forward(a2_)) { }
		Vertex() = default;
		A0 a0;
		A1 a1;
		A2 a2;
	};
	static_assert(sizeof(Vertex) == sizeof(A0) + sizeof(A1) + sizeof(A2), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		if (idx == 0) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A0 >::size,
				GLTypeInfo< A0 >::type,
				GLTypeInfo< A0 >::interpretation,
				sizeof(Vertex),
				0
			);
		} else if (idx == 1) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A1 >::size,
				GLTypeInfo< A1 >::type,
				GLTypeInfo< A1 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0)
			);
		} else if (idx == 2) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A2 >::size,
				GLTypeInfo< A2 >::type,
				GLTypeInfo< A2 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1)
			);
		} else {
			assert(idx < 3);
			return GLAttribPointer();
		}
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};

template< typename A0, typename A1, typename A2, typename A3 >
struct GLAttribBuffer< A0, A1, A2, A3 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_, A1 &&a1_, A2 &&a2_, A3 &&a3_) : a0(std::forward(a0_)), a1(std::forward(a1_)), a2(std::forward(a2_)), a3(std::forward(a3_)) { }
		Vertex() = default;
		A0 a0;
		A1 a1;
		A2 a2;
		A3 a3;
	};
	static_assert(sizeof(Vertex) == sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		if (idx == 0) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A0 >::size,
				GLTypeInfo< A0 >::type,
				GLTypeInfo< A0 >::interpretation,
				sizeof(Vertex),
				0
			);
		} else if (idx == 1) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A1 >::size,
				GLTypeInfo< A1 >::type,
				GLTypeInfo< A1 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0)
			);
		} else if (idx == 2) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A2 >::size,
				GLTypeInfo< A2 >::type,
				GLTypeInfo< A2 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1)
			);
		} else if (idx == 3) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A3 >::size,
				GLTypeInfo< A3 >::type,
				GLTypeInfo< A3 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2)
			);
		} else {
			assert(idx < 4);
			return GLAttribPointer();
		}
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};

template< typename A0, typename A1, typename A2, typename A3, typename A4 >
struct GLAttribBuffer< A0, A1, A2, A3, A4 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_, A1 &&a1_, A2 &&a2_, A3 &&a3_, A4 &&a4_) : a0(std::forward(a0_)), a1(std::forward(a1_)), a2(std::forward(a2_)), a3(std::forward(a3_)), a4(std::forward(a4_)) { }
		Vertex() = default;
		A0 a0;
		A1 a1;
		A2 a2;
		A3 a3;
		A4 a4;
	};
	static_assert(sizeof(Vertex) == sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3) + sizeof(A4), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		if (idx == 0) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A0 >::size,
				GLTypeInfo< A0 >::type,
				GLTypeInfo< A0 >::interpretation,
				sizeof(Vertex),
				0
			);
		} else if (idx == 1) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A1 >::size,
				GLTypeInfo< A1 >::type,
				GLTypeInfo< A1 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0)
			);
		} else if (idx == 2) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A2 >::size,
				GLTypeInfo< A2 >::type,
				GLTypeInfo< A2 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1)
			);
		} else if (idx == 3) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A3 >::size,
				GLTypeInfo< A3 >::type,
				GLTypeInfo< A3 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2)
			);
		} else if (idx == 4) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A4 >::size,
				GLTypeInfo< A4 >::type,
				GLTypeInfo< A4 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3)
			);
		} else {
			assert(idx < 5);
			return GLAttribPointer();
		}
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};

template< typename A0, typename A1, typename A2, typename A3, typename A4, typename A5 >
struct GLAttribBuffer< A0, A1, A2, A3, A4, A5 > : GLBuffer {
	GLsizei count = 0;

	struct Vertex {
		Vertex(A0 &&a0_, A1 &&a1_, A2 &&a2_, A3 &&a3_, A4 &&a4_, A5 &&a5_) : a0(std::forward(a0_)), a1(std::forward(a1_)), a2(std::forward(a2_)), a3(std::forward(a3_)), a4(std::forward(a4_)), a5(std::forward(a5_)) { }
		Vertex() = default;
		A0 a0;
		A1 a1;
		A2 a2;
		A3 a3;
		A4 a4;
		A4 a5;
	};
	static_assert(sizeof(Vertex) == sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3) + sizeof(A4) + sizeof(A5), "Vertex is packed.");

	GLAttribPointer operator[](uint32_t idx) const {
		if (idx == 0) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A0 >::size,
				GLTypeInfo< A0 >::type,
				GLTypeInfo< A0 >::interpretation,
				sizeof(Vertex),
				0
			);
		} else if (idx == 1) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A1 >::size,
				GLTypeInfo< A1 >::type,
				GLTypeInfo< A1 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0)
			);
		} else if (idx == 2) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A2 >::size,
				GLTypeInfo< A2 >::type,
				GLTypeInfo< A2 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1)
			);
		} else if (idx == 3) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A3 >::size,
				GLTypeInfo< A3 >::type,
				GLTypeInfo< A3 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2)
			);
		} else if (idx == 4) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A4 >::size,
				GLTypeInfo< A4 >::type,
				GLTypeInfo< A4 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3)
			);
		} else if (idx == 5) {
			return GLAttribPointer(buffer,
				GLTypeInfo< A5 >::size,
				GLTypeInfo< A5 >::type,
				GLTypeInfo< A5 >::interpretation,
				sizeof(Vertex),
				0 + sizeof(A0) + sizeof(A1) + sizeof(A2) + sizeof(A3) + sizeof(A4)
			);
		} else {
			assert(idx < 6);
			return GLAttribPointer();
		}
	};

	void set(GLsizei count_, Vertex const *data, GLenum usage) {
		count = count_;
		GLBuffer::set(GL_ARRAY_BUFFER, count_ * sizeof(Vertex), data, usage);
	}
	void set(std::vector< Vertex > const &data, GLenum usage) {
		set(data.size(), &data[0], usage);
	}
};


