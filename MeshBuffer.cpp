#include "MeshBuffer.hpp"
#include "read_chunk.hpp"

#include "GLBuffer.hpp"

#include <glm/glm.hpp>

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

namespace kit {

MeshBuffer::MeshBuffer(std::string const &filename) {
	std::ifstream file(filename, std::ios::binary);

	auto endswith = [&filename](std::string ext) {
		return filename.size() >= ext.size() && filename.substr(filename.size()-ext.size()) == ext;
	};

	GLuint total = 0;
	//read + upload data chunk:
	if (endswith(".p") || endswith(".pl")) {
		GLAttribBuffer< glm::vec3 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "p...", &data);

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);

		total = data.size(); //store total for later checks on index

		//store attrib locations:
		Position = buffer[0];

		this->buffer = std::move(buffer);
	} else if (endswith(".pn")) {
		GLAttribBuffer< glm::vec3, glm::vec3 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "pn..", &data);

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);

		total = data.size(); //store total for later checks on index

		//store attrib locations:
		Position = buffer[0];
		Normal = buffer[1];

		this->buffer = std::move(buffer);
	} else if (endswith(".pc")) {
		GLAttribBuffer< glm::vec3, glm::u8vec4 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "pc..", &data);

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);

		total = data.size(); //store total for later checks on index

		//store attrib locations:
		Position = buffer[0];
		Color = buffer[1];

		this->buffer = std::move(buffer);
	} else if (endswith(".pnc")) {
		GLAttribBuffer< glm::vec3, glm::vec3, glm::u8vec4 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "pnc.", &data);

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);

		total = data.size(); //store total for later checks on index

		//store attrib locations:
		Position = buffer[0];
		Normal = buffer[1];
		Color = buffer[2];

		this->buffer = std::move(buffer);
	} else if (endswith(".pnct")) {
		GLAttribBuffer< glm::vec3, glm::vec3, glm::u8vec4, glm::vec2 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "pnct", &data);

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);

		total = data.size(); //store total for later checks on index

		//store attrib locations:
		Position = buffer[0];
		Normal = buffer[1];
		Color = buffer[2];
		TexCoord = buffer[3];

		this->buffer = std::move(buffer);
	} else {
		throw std::runtime_error("Unknown file type '" + filename + "'");
	}
	assert(this->buffer.buffer);


	std::vector< char > strings;
	read_chunk(file, "str0", &strings);

	{ //read index chunk, add to meshes:
		struct IndexEntry {
			uint32_t name_begin, name_end;
			uint32_t vertex_start, vertex_count;
		};
		static_assert(sizeof(IndexEntry) == 16, "Index entry should be packed");

		std::vector< IndexEntry > index;
		read_chunk(file, "idx0", &index);

		for (auto const &entry : index) {
			if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings.size())) {
				throw std::runtime_error("index entry has out-of-range name begin/end");
			}
			if (!(entry.vertex_start < entry.vertex_start + entry.vertex_count && entry.vertex_start + entry.vertex_count <= total)) {
				throw std::runtime_error("index entry has out-of-range vertex start/count");
			}
			std::string name(&strings[0] + entry.name_begin, &strings[0] + entry.name_end);
			Mesh mesh;
			mesh.start = entry.vertex_start;
			mesh.count = entry.vertex_count;
			bool inserted = meshes.insert(std::make_pair(name, mesh)).second;
			if (!inserted) {
				std::cerr << "WARNING: mesh name '" + name + "' in filename '" + filename + "' collides with existing mesh." << std::endl;
			}
		}
	}

	if (file.peek() != EOF) {
		std::cerr << "WARNING: trailing data in mesh file '" + filename + "'" << std::endl;
	}
}

const MeshBuffer::Mesh &MeshBuffer::lookup(std::string const &name) const {
	auto f = meshes.find(name);
	if (f == meshes.end()) {
		throw std::runtime_error("Looking up mesh '" + name + "' that doesn't exist.");
	}
	return f->second;
}

}
