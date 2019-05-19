#pragma once

#include <string>
#include <vector>
#include <stdint.h>

/*
 * Load and save JPEG files.
 */

#ifndef LOAD_SAVE_ORIGIN
#define LOAD_SAVE_ORIGIN
enum OriginLocation {
	LowerLeftOrigin,
	UpperLeftOrigin,
};
#endif

//For convenience, data is returned as RGBA, just like load_save_png, even though jpeg doesn't contain alpha channel.

bool load_jpeg(std::string filename, unsigned int *width, unsigned int *height, std::vector< uint32_t > *data, OriginLocation origin);

//TODO:
//void save_jpeg(std::string filename, unsigned int width, unsigned int height, uint32_t const *data, OriginLocation origin);

/*
bool load_jpeg(std::istream &from, unsigned int *width, unsigned int *height, std::vector< uint32_t > *data, OriginLocation origin = UpperLeftOrigin);
void save_jpeg(std::ostream &to, unsigned int width, unsigned int height, uint32_t const *data, OriginLocation origin = UpperLeftOrigin);
*/
