#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "BoundingBox.h"

struct QuadricData {
	std::vector<glm::vec4> positions{};
	std::vector<glm::vec4> normals{};
	std::vector<glm::vec4> colors{};
	std::vector<glm::vec2> texcoords{};
	std::vector<uint32_t> indices;
	ss::BoundingBox bbox;
};

class QuadricDataFactory {
	const static float RS_PI;
public:
	static QuadricData createSphere(float radius, uint32_t numslices, uint32_t numstacks);
	static QuadricData createQuad(float size);
};