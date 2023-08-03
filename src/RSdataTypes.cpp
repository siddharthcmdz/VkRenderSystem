#include "RSdataTypes.h"
#include <glm/glm.hpp>
#include <stdexcept>

uint32_t RSvertexAttribsInfo::sizeOfAttrib() const {
	if (attributes == nullptr) {
		return 0;
	}

	uint32_t sum = 0;
	for (uint32_t i = 0; i < numVertexAttribs; i++) {
		switch (attributes[i]) {
		case RSvertexAttribute::vaPosition:
			sum += sizeof(glm::vec4);
			break;

		case RSvertexAttribute::vaColor:
			sum += sizeof(glm::vec4);
			break;

		case RSvertexAttribute::vaNormal:
			sum += sizeof(glm::vec4);
			break;

		case RSvertexAttribute::vaTexCoord:
			sum += sizeof(glm::vec2);
			break;
		}
	}

	if (sum == 0) {
		throw std::runtime_error("size of one attribute cannot be zero!");
	}

	return sum;
}