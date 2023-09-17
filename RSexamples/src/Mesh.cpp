#include "Mesh.h"

namespace ss {

	Mesh::Mesh(const std::vector<glm::vec4> vertices, std::vector<uint32_t> indices) {
		this->ivertices = vertices;
		this->iindices = indices;
	}
	
	std::vector<glm::vec4> Mesh::getVertices() const {
		return ivertices;
	}

	std::vector<uint32_t> Mesh::getIndices() const {
		return iindices;
	}
}

