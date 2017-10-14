#pragma once

#include "gl.hpp"

#include <glm/glm.hpp>

//Traits struct that stores GL info about c++/glm types:
template< typename T >
struct GLTypeInfo;

#define SPECIALIZE( GLM_TYPE, GL_SIZE, GL_TYPE, GL_NORMALIZED ) \
	template< > \
	struct GLTypeInfo< GLM_TYPE > { \
		enum : GLint { size = GL_SIZE }; \
		enum : GLenum { type = GL_TYPE }; \
		enum : GLboolean { normalized = GL_NORMALIZED }; \
	};

SPECIALIZE( float, 1, GL_FLOAT, GL_FALSE );
SPECIALIZE( glm::vec2, 2, GL_FLOAT, GL_FALSE );
SPECIALIZE( glm::vec3, 3, GL_FLOAT, GL_FALSE );
SPECIALIZE( glm::vec4, 4, GL_FLOAT, GL_FALSE );
SPECIALIZE( glm::u8vec3, 3, GL_UNSIGNED_BYTE, GL_TRUE );
SPECIALIZE( glm::u8vec4, 4, GL_UNSIGNED_BYTE, GL_TRUE );

#undef SPECIALIZE

