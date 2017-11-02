#pragma once

#include "gl.hpp"

#include <glm/glm.hpp>

//Traits struct that stores GL info about c++/glm types:
template< typename T >
struct GLTypeInfo;

enum : uint8_t {
	KIT_AS_INTEGER = 0,
	KIT_AS_FLOAT = 1,
	KIT_NORMALIZED_AS_FLOAT = 2,
	KIT_AS_DOUBLE = 3,
};

#define SPECIALIZE( GLM_TYPE, GL_SIZE, GL_TYPE, KIT_INTERPRETATION ) \
	template< > \
	struct GLTypeInfo< GLM_TYPE > { \
		enum : GLint { size = GL_SIZE }; \
		enum : GLenum { type = GL_TYPE }; \
		enum : uint8_t { interpretation = KIT_INTERPRETATION }; \
	};

SPECIALIZE( float, 1, GL_FLOAT, KIT_AS_FLOAT );
SPECIALIZE( glm::vec2, 2, GL_FLOAT, KIT_AS_FLOAT );
SPECIALIZE( glm::vec3, 3, GL_FLOAT, KIT_AS_FLOAT );
SPECIALIZE( glm::vec4, 4, GL_FLOAT, KIT_AS_FLOAT );

SPECIALIZE( uint8_t, 1, GL_UNSIGNED_BYTE, KIT_NORMALIZED_AS_FLOAT );
SPECIALIZE( glm::u8vec2, 2, GL_UNSIGNED_BYTE, KIT_NORMALIZED_AS_FLOAT );
SPECIALIZE( glm::u8vec3, 3, GL_UNSIGNED_BYTE, KIT_NORMALIZED_AS_FLOAT );
SPECIALIZE( glm::u8vec4, 4, GL_UNSIGNED_BYTE, KIT_NORMALIZED_AS_FLOAT );

SPECIALIZE( uint32_t, 1, GL_UNSIGNED_INT, KIT_AS_INTEGER );
SPECIALIZE( glm::uvec2, 2, GL_UNSIGNED_INT, KIT_AS_INTEGER );
SPECIALIZE( glm::uvec3, 3, GL_UNSIGNED_INT, KIT_AS_INTEGER );
SPECIALIZE( glm::uvec4, 4, GL_UNSIGNED_INT, KIT_AS_INTEGER );

#undef SPECIALIZE

