#pragma once

#include <glm/glm.hpp>

#include "rsenums.h"

namespace rsvd {
	struct Vertex {
		glm::vec4 pos;
		glm::vec4 color;
	};

	struct Vertex3 {
		glm::vec4 pos;
		glm::vec4 color;
		glm::vec4 normal;
	};
}
