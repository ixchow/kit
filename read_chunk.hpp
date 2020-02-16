#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>

template< typename T >
void read_chunk(std::istream &from, std::string const &magic, std::vector< T > *to_) {
	assert(to_);
	auto &to = *to_;

	struct ChunkHeader {
		char magic[4] = {'\0', '\0', '\0', '\0'};
		uint32_t size = 0;
	};
	static_assert(sizeof(ChunkHeader) == 8, "header is packed");

	ChunkHeader header;
	if (!from.read(reinterpret_cast< char * >(&header), sizeof(header))) {
		throw std::runtime_error("Failed to read chunk header");
	}
	if (std::string(header.magic,4) != magic) {
		throw std::runtime_error("Unexpected magic number in chunk");
	}

	if (header.size % sizeof(T) != 0) {
		throw std::runtime_error("Size of chunk not divisible by element size");
	}

	to.resize(header.size / sizeof(T));
	if (!from.read(reinterpret_cast< char * >(&to[0]), to.size() * sizeof(T))) {
		throw std::runtime_error("Failed to read chunk data.");
	}
}

template< typename T >
void read_struct(std::istream &from, std::string const &magic, T *to_) {
	assert(to_);
	auto &to = *to_;

	struct ChunkHeader {
		char magic[4] = {'\0', '\0', '\0', '\0'};
		uint32_t size = 0;
	};
	static_assert(sizeof(ChunkHeader) == 8, "header is packed");

	ChunkHeader header;
	if (!from.read(reinterpret_cast< char * >(&header), sizeof(header))) {
		throw std::runtime_error("Failed to read chunk header");
	}
	if (std::string(header.magic,4) != magic) {
		throw std::runtime_error("Unexpected magic number in chunk");
	}

	if (header.size != sizeof(T)) {
		throw std::runtime_error("Size of chunk not struct size");
	}

	if (!from.read(reinterpret_cast< char * >(&to), sizeof(T))) {
		throw std::runtime_error("Failed to read struct chunk data.");
	}
}

template< typename T >
void write_chunk(std::string const &magic, std::vector< T > const &from, std::ostream *to_) {
	assert(magic.size() == 4);
	assert(to_);
	auto &to = *to_;

	struct ChunkHeader {
		char magic[4] = {'\0', '\0', '\0', '\0'};
		uint32_t size = 0;
	};
	static_assert(sizeof(ChunkHeader) == 8, "header is packed");
	ChunkHeader header;
	header.magic[0] = magic[0];
	header.magic[1] = magic[1];
	header.magic[2] = magic[2];
	header.magic[3] = magic[3];
	header.size = from.size() * sizeof(T);

	to.write(reinterpret_cast< const char * >(&header), sizeof(header));
	to.write(reinterpret_cast< const char * >(from.data()), from.size() * sizeof(T));
}


template< typename T >
void write_struct(std::string const &magic, T const &from, std::ostream *to_) {
	assert(magic.size() == 4);
	assert(to_);
	auto &to = *to_;

	struct ChunkHeader {
		char magic[4] = {'\0', '\0', '\0', '\0'};
		uint32_t size = 0;
	};
	static_assert(sizeof(ChunkHeader) == 8, "header is packed");
	ChunkHeader header;
	header.magic[0] = magic[0];
	header.magic[1] = magic[1];
	header.magic[2] = magic[2];
	header.magic[3] = magic[3];
	header.size = sizeof(T);

	to.write(reinterpret_cast< const char * >(&header), sizeof(header));
	to.write(reinterpret_cast< const char * >(&from), sizeof(T));
}
