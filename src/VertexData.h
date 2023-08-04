#pragma once

#include <glm/glm.hpp>

namespace rsvd {
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
	};

	struct Vertex3 {
		glm::vec4 pos;
		glm::vec4 color;
		glm::vec4 normal;
	};
}
