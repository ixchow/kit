#pragma once

/*
 * GLTexture wraps a handle to an OpenGL texture.
 *
 */

#include "gl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GLTexture {
	GLuint texture = 0;
	GLTexture() { glGenTextures(1, &texture); }
	~GLTexture() { if (texture != 0) glDeleteTextures(1, &texture); }
	GLTexture(GLTexture const &) = delete;
	GLTexture(GLTexture &&from) { std::swap(texture, from.texture); }
	GLTexture &operator=(GLTexture &&from) { std::swap(texture, from.texture); return *this; }

	//helper to call TexImage:
	void set(glm::uvec2 size, std::vector< glm::u8vec4 > const &data) {
		assert(size.x * size.y == data.size());
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(data[0]));
		glBindTexture(GL_TEXTURE_2D, 0);
	}

};
