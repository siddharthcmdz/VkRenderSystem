#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace ss {

	enum TextureType {
		ttAmbient,
		ttDiffuse,
		ttSpecular,
		ttNormal,
		ttRoughness,
		ttMax
	};

	struct Texture {
		unsigned char* texels = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		std::string path;
		TextureType textype;
	};

	class Material {
	private:
		std::vector<Texture> textureList;
		glm::vec4 ambient, diffuse, specular;

	public:
		Material() = default;

	};
}
