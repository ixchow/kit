#pragma once

#include <string>

#include <kit.hpp>

//load an (rgbe) cubemap texture:
// returns a freshly allocated cube map texture on success
// throws on failure
GLuint load_cube(std::string const &filename);
