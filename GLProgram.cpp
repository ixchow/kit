#include "GLProgram.hpp"

#include <iostream>
#include <vector>

GLShader::GLShader( GLenum type, std::string const &source ) {
	shader = glCreateShader(type);
	{
		GLchar const *str = source.c_str();
		GLint length = source.size();
		glShaderSource(shader, 1, &str, &length);
	}
	glCompileShader(shader);
	GLint compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		std::cerr << "Failed to compile shader." << std::endl;
		GLint info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetShaderInfoLog(shader, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		glDeleteShader(shader);
		throw std::runtime_error("Failed to compile shader.");
	}
}

GLProgram::GLProgram( std::initializer_list< GLShader const * > shaders ) {
	program = glCreateProgram();
	for (auto s : shaders) {
		glAttachShader(program, s->shader);
	}
	glLinkProgram(program);
	GLint link_status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		std::cerr << "Failed to link shader program." << std::endl;
		GLint info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetProgramInfoLog(program, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		throw std::runtime_error("Failed to link program");
	}
}

void GLProgram::DEBUG_dump_info(std::string const &name_) {
	std::cout << " ------ Shader Program '" << name_ << "' -----\n";
	{
		GLint active = 0;
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &active);
		std::cout << "  " << active << " active attributes:\n";
		for (GLuint l = 0; l < (GLuint)active; ++l) {
			GLchar name[100];
			GLint size = 0;
			GLenum type = 0;
			glGetActiveAttrib(program, l, 100, NULL, &size, &type, name);
			name[99] = '\0';
			GLuint idx = glGetAttribLocation(program, name);
			auto str = [](GLenum type) -> std::string {
				#undef DO
				#define DO( X ) if (type == GL_ ## X) return #X;
				DO( FLOAT ) DO( FLOAT_VEC2 ) DO( FLOAT_VEC3 ) DO( FLOAT_VEC4 )
				DO( FLOAT_MAT2 ) DO( FLOAT_MAT2x3 ) DO( FLOAT_MAT2x4 )
				DO( FLOAT_MAT3x2 ) DO( FLOAT_MAT3 ) DO( FLOAT_MAT3x4 )
				DO( FLOAT_MAT4x2 ) DO( FLOAT_MAT4x3 ) DO( FLOAT_MAT4 )
				DO( INT ) DO( INT_VEC2 ) DO( INT_VEC3 ) DO( INT_VEC4 )
				DO( UNSIGNED_INT ) DO( UNSIGNED_INT_VEC2 ) DO( UNSIGNED_INT_VEC3 ) DO( UNSIGNED_INT_VEC4 )
				DO( DOUBLE ) DO( DOUBLE_VEC2 ) DO( DOUBLE_VEC3 ) DO( DOUBLE_VEC4 )
				DO( DOUBLE_MAT2 ) DO( DOUBLE_MAT2x3 ) DO( DOUBLE_MAT2x4 )
				DO( DOUBLE_MAT3x2 ) DO( DOUBLE_MAT3 ) DO( DOUBLE_MAT3x4 )
				DO( DOUBLE_MAT4x2 ) DO( DOUBLE_MAT4x3 ) DO( DOUBLE_MAT4 )
				#undef DO
				return "[unknown type: " + std::to_string(type);
			};
			std::cout << "   (" << idx << ") " << name << " [" << str(type) << " x " << size << "]\n";
		}
	}
	{
		GLint active = 0;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &active);
		std::cout << "  " << active << " active uniforms:\n";
		for (GLuint l = 0; l < (GLuint)active; ++l) {
			GLchar name[100];
			GLint size = 0;
			GLenum type = 0;
			glGetActiveUniform(program, l, 100, NULL, &size, &type, name);
			name[99] = '\0';
			GLuint idx = glGetUniformLocation(program, name);
			auto str = [](GLenum type) -> std::string {
				#define DO( X ) if (type == GL_ ## X) return #X;
				DO( FLOAT ) DO( FLOAT_VEC2 ) DO( FLOAT_VEC3 ) DO( FLOAT_VEC4 )
				DO( DOUBLE ) DO( DOUBLE_VEC2 ) DO( DOUBLE_VEC3 ) DO( DOUBLE_VEC4 )
				DO( INT ) DO( INT_VEC2 ) DO( INT_VEC3 ) DO( INT_VEC4 )
				DO( UNSIGNED_INT ) DO( UNSIGNED_INT_VEC2 ) DO( UNSIGNED_INT_VEC3 ) DO( UNSIGNED_INT_VEC4 )
				DO( BOOL ) DO( BOOL_VEC2 ) DO( BOOL_VEC3 ) DO( BOOL_VEC4 )
				DO( FLOAT_MAT2 ) DO( FLOAT_MAT2x3 ) DO( FLOAT_MAT2x4 )
				DO( FLOAT_MAT3x2 ) DO( FLOAT_MAT3 ) DO( FLOAT_MAT3x4 )
				DO( FLOAT_MAT4x2 ) DO( FLOAT_MAT4x3 ) DO( FLOAT_MAT4 )
				DO( DOUBLE_MAT2 ) DO( DOUBLE_MAT2x3 ) DO( DOUBLE_MAT2x4 )
				DO( DOUBLE_MAT3x2 ) DO( DOUBLE_MAT3 ) DO( DOUBLE_MAT3x4 )
				DO( DOUBLE_MAT4x2 ) DO( DOUBLE_MAT4x3 ) DO( DOUBLE_MAT4 )
				DO( SAMPLER_1D ) DO( SAMPLER_2D ) DO( SAMPLER_3D ) DO( SAMPLER_CUBE )
				DO( SAMPLER_1D_SHADOW ) DO( SAMPLER_2D_SHADOW )
				DO( SAMPLER_1D_ARRAY ) DO( SAMPLER_2D_ARRAY )
				DO( SAMPLER_1D_ARRAY_SHADOW ) DO( SAMPLER_2D_ARRAY_SHADOW )
				DO( SAMPLER_2D_MULTISAMPLE ) DO( SAMPLER_2D_MULTISAMPLE_ARRAY )
				DO( SAMPLER_CUBE_SHADOW )
				DO( SAMPLER_BUFFER )
				DO( SAMPLER_2D_RECT ) DO( SAMPLER_2D_RECT_SHADOW )

				DO( INT_SAMPLER_1D ) DO( INT_SAMPLER_2D ) DO( INT_SAMPLER_3D ) DO( INT_SAMPLER_CUBE )
				DO( INT_SAMPLER_1D_ARRAY ) DO( INT_SAMPLER_2D_ARRAY )
				DO( INT_SAMPLER_2D_MULTISAMPLE ) DO( INT_SAMPLER_2D_MULTISAMPLE_ARRAY )
				DO( INT_SAMPLER_BUFFER ) DO( INT_SAMPLER_2D_RECT )

				DO( UNSIGNED_INT_SAMPLER_1D ) DO( UNSIGNED_INT_SAMPLER_2D ) DO( UNSIGNED_INT_SAMPLER_3D ) DO( UNSIGNED_INT_SAMPLER_CUBE )
				DO( UNSIGNED_INT_SAMPLER_1D_ARRAY ) DO( UNSIGNED_INT_SAMPLER_2D_ARRAY )
				DO( UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE ) DO( UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY )
				DO( UNSIGNED_INT_SAMPLER_BUFFER ) DO( UNSIGNED_INT_SAMPLER_2D_RECT )

				DO( IMAGE_1D ) DO( IMAGE_2D ) DO( IMAGE_3D )
				DO( IMAGE_2D_RECT ) DO( IMAGE_CUBE ) DO( IMAGE_BUFFER )
				DO( IMAGE_1D_ARRAY ) DO( IMAGE_2D_ARRAY )
				DO( IMAGE_2D_MULTISAMPLE ) DO( IMAGE_2D_MULTISAMPLE_ARRAY )

				DO( INT_IMAGE_1D ) DO( INT_IMAGE_2D ) DO( INT_IMAGE_3D )
				DO( INT_IMAGE_2D_RECT ) DO( INT_IMAGE_CUBE ) DO( INT_IMAGE_BUFFER )
				DO( INT_IMAGE_1D_ARRAY ) DO( INT_IMAGE_2D_ARRAY )
				DO( INT_IMAGE_2D_MULTISAMPLE ) DO( INT_IMAGE_2D_MULTISAMPLE_ARRAY )

				DO( UNSIGNED_INT_IMAGE_1D ) DO( UNSIGNED_INT_IMAGE_2D ) DO( UNSIGNED_INT_IMAGE_3D )
				DO( UNSIGNED_INT_IMAGE_2D_RECT ) DO( UNSIGNED_INT_IMAGE_CUBE ) DO( UNSIGNED_INT_IMAGE_BUFFER )
				DO( UNSIGNED_INT_IMAGE_1D_ARRAY ) DO( UNSIGNED_INT_IMAGE_2D_ARRAY )
				DO( UNSIGNED_INT_IMAGE_2D_MULTISAMPLE ) DO( UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY )

				DO( UNSIGNED_INT_ATOMIC_COUNTER )
				#undef DO

				return "[unknown type: " + std::to_string(type);
			};

			std::cout << "   (" << idx << ") " << name << " [" << str(type) << " x " << size << "]\n";
		}
	}



	std::cout << " -------------------------------------\n";

}
