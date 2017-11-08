#include "BoneAnimation.hpp"

#include "read_chunk.hpp"

#include <fstream>

kit::BoneAnimation::BoneAnimation(std::string const &filename) {
	std::cout << "Reading bone-based animation from '" << filename << "'." << std::endl;

	std::ifstream file(filename, std::ios::binary);

	std::vector< char > strings;
	read_chunk(file, "str0", &strings);

	{ //read bones:
		struct BoneInfo {
			uint32_t name_begin, name_end;
			uint32_t parent;
			glm::mat4x3 inverse_bind_matrix;
		};
		static_assert(sizeof(BoneInfo) == 4*2 + 4 + 4*12, "BoneInfo is packed.");

		std::vector< BoneInfo > file_bones;
		read_chunk(file, "bon0", &file_bones);
		bones.reserve(file_bones.size());
		for (auto const &file_bone : file_bones) {
			if (!(file_bone.name_begin <= file_bone.name_end && file_bone.name_end <= strings.size())) {
				throw std::runtime_error("bone has out-of-range name begin/end");
			}
			if (!(file_bone.parent == -1U || file_bone.parent < bones.size())) {
				throw std::runtime_error("bone has invalid parent");
			}
			bones.emplace_back();
			Bone &bone = bones.back();
			bone.name = std::string(&strings[0] + file_bone.name_begin, &strings[0] + file_bone.name_end);
			bone.parent = file_bone.parent;
			bone.inverse_bind_matrix = file_bone.inverse_bind_matrix;
		}
	}

	static_assert(sizeof(PoseBone) == 3*4 + 4*4 + 3*4, "PoseBone is packed.");
	read_chunk(file, "frm0", &frame_bones);
	if (frame_bones.size() % bones.size() != 0) {
		throw std::runtime_error("frame bones is not divisible by bones");
	}

	uint32_t frames = frame_bones.size() / bones.size();

	{ //read actions (animations):
		struct AnimationInfo {
			uint32_t name_begin, name_end;
			uint32_t begin, end;
		};
		static_assert(sizeof(AnimationInfo) == 4*2 + 4*2, "AnimationInfo is packed.");

		std::vector< AnimationInfo > file_animations;
		read_chunk(file, "act0", &file_animations);
		animations.reserve(file_animations.size());
		for (auto const &file_animation : file_animations) {
			if (!(file_animation.name_begin <= file_animation.name_end && file_animation.name_end <= strings.size())) {
				throw std::runtime_error("animation has out-of-range name begin/end");
			}
			if (!(file_animation.begin <= file_animation.end && file_animation.end <= frames)) {
				throw std::runtime_error("animation has out-of-range frames begin/end");
			}
			animations.emplace_back();
			Animation &animation = animations.back();
			animation.name = std::string(&strings[0] + file_animation.name_begin, &strings[0] + file_animation.name_end);
			animation.begin = file_animation.begin;
			animation.end = file_animation.end;
		}
	}

	{ //read actual mesh:
		GLAttribBuffer< glm::vec3, glm::vec3, glm::u8vec4, glm::vec2, glm::vec4, glm::uvec4 > buffer;
		std::vector< decltype(buffer)::Vertex > data;
		read_chunk(file, "msh0", &data);

		//check bone indices:
		for (auto const &vertex : data) {
			if ( vertex.a5.x >= bones.size()
				|| vertex.a5.y >= bones.size()
				|| vertex.a5.z >= bones.size()
				|| vertex.a5.w >= bones.size()
			) {
				throw std::runtime_error("animation mesh has out of range vertex index");
			}
		}

		{ //DEBUG: dump bounding box info
			glm::vec3 min, max;
			min = max = data[0].a0;
			for (auto const &v : data) {
				min = glm::min(min, v.a0);
				max = glm::max(max, v.a0);
			}
			std::cout << "INFO: bounding box of animation mesh in '" << filename << "' is [" << min.x << "," << max.x << "]x[" << min.y << "," << max.y << "]x[" << min.z << "," << max.z << "]" << std::endl;
		}

		//upload data:
		buffer.set(data, GL_STATIC_DRAW);
		vertex_count = data.size();

		Position = buffer[0];
		Normal = buffer[1];
		Color = buffer[2];
		TexCoord = buffer[3];
		BoneWeights = buffer[4];
		BoneIndices = buffer[5];

		this->buffer = std::move(buffer);
	}

}

const kit::BoneAnimation::Animation &kit::BoneAnimation::lookup(std::string const &name) const {
	for (auto const &animation : animations) {
		if (animation.name == name) return animation;
	}
	throw std::runtime_error("Animation with name '" + name + "' does not exist.");
}

