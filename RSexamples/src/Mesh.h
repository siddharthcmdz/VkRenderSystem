#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace ss {
	class Mesh {
	private:
		std::vector<glm::vec4> ivertices;
		std::vector<uint32_t> iindices;

	public:
		Mesh(const std::vector<glm::vec4> vertices, std::vector<uint32_t> indices);
		std::vector<glm::vec4> getVertices() const;
		std::vector<uint32_t> getIndices() const;
	};
}