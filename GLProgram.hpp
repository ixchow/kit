#pragma once

/*
 * GLShader is a wrapper for an OpenGL Shader Object. (Useful when sharing between programs.)
 *
 * GLProgram is a wrapper for an OpenGL Shader Program.
 * It provides some convenience methods to perform *checked*
 *  lookups of attributes and uniforms.
 */

#include "gl.hpp"

#include <string>
#include <stdexcept>
#include <iostream>

struct GLShader {
	GLuint shader = 0;

	//Compile a shader from source:
	GLShader( GLenum type, std::string const &source );

	~GLShader() { if (shader != 0) glDeleteShader(shader); }
	GLShader(GLShader const &) = delete;
	GLShader(GLShader &&from) { std::swap(shader, from.shader); }
	GLShader &operator=(GLShader &&from) { std::swap(shader, from.shader); return *this; }
};

struct GLProgram {
	GLuint program = 0;

	//Compiles + links program from source; throws on compile error:
	GLProgram(
		std::string const &vertex_source,
		std::string const &fragment_source
	) : GLProgram(
		GLShader( GL_VERTEX_SHADER, vertex_source ),
		GLShader( GL_FRAGMENT_SHADER, fragment_source ) ) { }

	//Links program from shader objects:
	GLProgram( GLShader const &shader0, GLShader const &shader1 );

	~GLProgram() { if (program != 0) glDeleteProgram(program); }
	GLProgram(GLProgram const &) = delete;
	GLProgram(GLProgram &&from) { std::swap(program, from.program); }
	GLProgram &operator=(GLProgram &&from) { std::swap(program, from.program); return *this; }

	//---------

	enum MissingIs {
		MissingIsWarning = 0,
		MissingIsError = 1,
	};

	//getAttribLocation looks up an attribute:
	GLint getAttribLocation(std::string const &name, MissingIs missing_is = MissingIsError) const {
		GLint ret = glGetAttribLocation(program, name.c_str());
		if (ret == -1) {
			if (missing_is == MissingIsWarning) {
				std::cerr << "WARNING: Attribute '" + name + "' does not exist in program." << std::endl;
			} else {
				throw std::runtime_error("Attribute '" + name + "' does not exist in program.");
			}
		}
		return ret;
	}

	//getUniformLocation looks up a uniform address:
	GLint getUniformLocation(std::string const &name, MissingIs missing_is = MissingIsError) const {
		GLint ret = glGetUniformLocation(program, name.c_str());
		if (ret == -1) {
			if (missing_is == MissingIsWarning) {
				std::cerr << "WARNING: Uniform '" + name + "' does not exist in program." << std::endl;
			} else {
				throw std::runtime_error("Uniform '" + name + "' does not exist in program.");
			}
		}
		return ret;
	}

	//shorthand for getUniformLocation / getAttribLocation:

	//operator() looks up an attribute (and checks that it exists):
	GLint operator()(std::string const &name) const {
		return getAttribLocation(name, MissingIsError);
	}

	//operator[] looks up a uniform address (and checks that it exists):
	GLint operator[](std::string const &name) const {
		return getUniformLocation(name, MissingIsError);
	}

};
