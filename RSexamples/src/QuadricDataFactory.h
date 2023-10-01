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
private:
	const static float RS_PI;
	const static float DEFAULT_NUM_STACKS;
	const static float DEFAULT_NUM_SLICES;
	const static float DEFAULT_RADIUS;
	const static float DEFAULT_PHI_MAX;
	const static float DEFAULT_SIZE;
	const static float DEFAULT_HEIGHT;
	const static float DEFAULT_HALF_RADIUS;

public:
	static QuadricData createSphere(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_STACKS, uint32_t numstacks = DEFAULT_NUM_SLICES);
	static QuadricData createQuad(float size = DEFAULT_SIZE);
	static QuadricData createCone(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float height = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
	static QuadricData createCylinder(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float zmin = 0, float zmax = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
	static QuadricData createDisk(float innerRadius = DEFAULT_HALF_RADIUS, float outerRadius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float z = 0, float phimax = DEFAULT_PHI_MAX);
};