#include "VkRSdataTypes.h"

VkFormat getVkFormat(const RSvertexAttribute& va) {
	switch (va) {
	case RSvertexAttribute::vaPosition:
	case RSvertexAttribute::vaColor:
	case RSvertexAttribute::vaNormal:
	case RSvertexAttribute::vaTexCoord:
		return VK_FORMAT_R32G32B32A32_SFLOAT;

	default:
		break;
	}

	VkFormat::VK_FORMAT_MAX_ENUM;
}

uint32_t getOffset(uint32_t numAttribs, uint32_t idx) {
	uint32_t curroffset = 0;
	for (size_t i = 0; i < numAttribs; i++) {
		if (idx == i) {
			return curroffset;
		}

		curroffset += sizeof(glm::vec4);
	}
}