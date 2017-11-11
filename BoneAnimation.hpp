#pragma once

#include "GLBuffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
#include <string>

//"BoneAnimation" holds a mesh loaded from a file along with skin weights,
// a heirarchy of bones and their bind info,
// and a collection of animations defined on those bones

namespace kit {

struct BoneAnimation {
	GLAttribPointer Position;
	GLAttribPointer Normal;
	GLAttribPointer Color;
	GLAttribPointer TexCoord;
	GLAttribPointer BoneWeights;
	GLAttribPointer BoneIndices;

	GLBuffer buffer;
	uint32_t vertex_count = 0;

	struct Bone {
		std::string name;
		uint32_t parent = -1U;
		glm::mat4x3 inverse_bind_matrix;
	};
	std::vector< Bone > bones;

	struct PoseBone {
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};
	std::vector< PoseBone > frame_bones;

	PoseBone const *get_frame(uint32_t frame) const {
		return &frame_bones[frame * bones.size()];
	}

	struct Animation {
		std::string name;
		uint32_t begin = 0;
		uint32_t end = 0;
	};

	std::vector< Animation > animations;

	//look up a particular animation, will throw if not found:
	const Animation &lookup(std::string const &name) const;

	//construct from a file:
	// note: will throw if file fails to read.
	BoneAnimation(std::string const &filename);
};

}
