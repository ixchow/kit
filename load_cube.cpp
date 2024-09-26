#include "load_cube.hpp"
#include "rgbe.hpp"
#include "load_save_png.hpp"
#include "gl_errors.hpp"

#include <stdexcept>

//load an rgbe cubemap texture:
GLuint load_cube(std::string const &filename) {
	//assume cube is stacked faces +x,-x,+y,-y,+z,-z:
	glm::uvec2 size;
	std::vector< uint32_t > data;
	if (!load_png(filename, &size.x, &size.y, &data, LowerLeftOrigin)) {
		throw std::runtime_error("Failed to load '" + filename + "' as png.");
	}
	if (size.y != size.x * 6) {
		throw std::runtime_error("Expecting stacked faces in cubemap.");
	}

	//convert from rgb+exponent to floating point:
	std::vector< glm::vec3 > float_data;
	float_data.reserve(data.size());
	for (auto const &px : data) {
		float_data.emplace_back(rgbe_to_float(*reinterpret_cast< glm::u8vec4 const * >(&px)));
	}

	//upload to cubemap:
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	//the RGB9_E5 format is close to the source format and a lot more efficient to store than full floating point.
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 0*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 1*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 2*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 3*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 4*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 5*size.x*size.x);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//this will probably be ignored because of GL_TEXTURE_CUBE_MAP_SEAMLESS:
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//NOTE: turning this on to enable nice filtering at cube map boundaries:
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	GL_ERRORS();

	return tex;
}


