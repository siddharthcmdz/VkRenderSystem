#pragma once

#include <glm/glm.hpp>

#include "rsenums.h"

namespace rsvd {
	struct VertexPC {
		glm::vec4 pos;
		glm::vec4 color;
	};

	struct VertexPCT {
		glm::vec4 pos;
		glm::vec4 color;
		glm::vec2 texcoord;
	};

	struct VertexPCN {
		glm::vec4 pos;
		glm::vec4 color;
		glm::vec4 normal;
	};

	struct VertexPCNT {
		glm::vec4 pos;
		glm::vec4 color;
		glm::vec4 normal;
		glm::vec2 texcoord;
	};
}
