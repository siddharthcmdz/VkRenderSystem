#include "QuadricDataFactory.h"
#include <math.h>

const float QuadricDataFactory::RS_PI = 3.14159265358979323846f;
const float QuadricDataFactory::DEFAULT_NUM_STACKS = 10.0f;
const float QuadricDataFactory::DEFAULT_NUM_SLICES = 10.0f;
const float QuadricDataFactory::DEFAULT_RADIUS = 0.5f;
const float QuadricDataFactory::DEFAULT_PHI_MAX = 2 * RS_PI;
const float QuadricDataFactory::DEFAULT_SIZE = 0.5f;
const float QuadricDataFactory::DEFAULT_HEIGHT = 0.5;
const float QuadricDataFactory::DEFAULT_HALF_RADIUS = QuadricDataFactory::DEFAULT_RADIUS * 0.5f;

QuadricData QuadricDataFactory::createSphere(float radius, uint32_t numslices, uint32_t numstacks) {
	QuadricData qd;
	//theta -> [0, pi] vertical plane
	//phi -> [0, 2pi] horizontal plane
	float phiMax = 2 * RS_PI;
	float thetaMin = 0.0f;
	float thetaMax = RS_PI;
	ss::BoundingBox bbox;
	for (uint32_t i = 0; i < numslices; i++) {
		for (uint32_t j = 0; j < numstacks; j++) {
			float u = float(i) / float(numslices-1);
			float v = float(j) / float(numstacks-1);
			
			float phi = u * phiMax;
			float theta = thetaMin + v * (thetaMax - thetaMin);

			float x = radius * sinf(theta) * cosf(phi);
			float y = radius * sinf(theta) * sinf(phi);
			float z = radius * cosf(theta);
			
			glm::vec4 position(x, y, z, 1.0f);
			glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
			glm::vec4 color(u, v, 0.5f, 1.0f);
			glm::vec2 texcoord(u, v);
			
			bbox.expandBy(position);

			qd.positions.push_back(position);
			qd.colors.push_back(color);
			qd.normals.push_back(normal);
			qd.texcoords.push_back(texcoord);

			if (i < (numslices - 1) && j < (numstacks - 1)) {
				uint32_t start = i * numstacks + j;
				
				uint32_t idx0 = start;
				uint32_t idx1 = start + numstacks;
				uint32_t idx2 = start + 1;

				uint32_t idx3 = start + 1;
				uint32_t idx4 = start + numstacks;
				uint32_t idx5 = start + numstacks + 1;

				qd.indices.push_back(idx0);
				qd.indices.push_back(idx1);
				qd.indices.push_back(idx2);
				qd.indices.push_back(idx3);
				qd.indices.push_back(idx4);
				qd.indices.push_back(idx5);
			}

			qd.bbox = bbox;
		}
	}

	return qd;
}

QuadricData QuadricDataFactory::createQuad(float size) {
	
	QuadricData qd;
	float halfsize = size / 2.f;
	qd.positions = std::vector<glm::vec4>{
		{-halfsize, -halfsize, 0.0f, 1.0f},
		{halfsize, -halfsize, 0.0f, 1.0f},
		{halfsize, halfsize, 0.0f, 1.0f},
		{-halfsize, halfsize, 0.0f, 1.0f}
	};

	qd.normals = std::vector<glm::vec4>{
		{1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f}
	};

	qd.colors = std::vector<glm::vec4>{
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 0.0f, 1.0f},
	};

	qd.texcoords = std::vector<glm::vec2>{
		{1.0f, 0.0f},
		{0.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 1.0f}
	};

	return qd;
}

QuadricData QuadricDataFactory::createCylinder(float radius, float numslices, float numstacks, float zmin, float zmax, float phimax) {
	QuadricData qd;
	float phiMax = phimax;
	ss::BoundingBox bbox;
	for (uint32_t i = 0; i < numslices; i++) {
		for (uint32_t j = 0; j < numstacks; j++) {
			float u = float(i) / float(numslices - 1);
			float v = float(j) / float(numstacks - 1);

			float phi = u * phiMax;
			
			float x = radius * cosf(phi);
			float y = radius * sinf(phi);
			float z = zmin + v * (zmax - zmin);

			glm::vec4 position(x, y, z, 1.0f);
			glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
			glm::vec4 color(u, v, 0.5f, 1.0f);
			glm::vec2 texcoord(u, v);

			qd.positions.push_back(position);
			qd.normals.push_back(normal);
			qd.colors.push_back(color);
			qd.texcoords.push_back(texcoord);
			bbox.expandBy(position);

			if (i < (numslices - 1) && j < (numstacks - 1)) {
				uint32_t start = i * numstacks + j;

				uint32_t idx0 = start;
				uint32_t idx1 = start + numstacks;
				uint32_t idx2 = start + 1;

				uint32_t idx3 = start + 1;
				uint32_t idx4 = start + numstacks;
				uint32_t idx5 = start + numstacks + 1;

				qd.indices.push_back(idx0);
				qd.indices.push_back(idx1);
				qd.indices.push_back(idx2);
				qd.indices.push_back(idx3);
				qd.indices.push_back(idx4);
				qd.indices.push_back(idx5);
			}

			qd.bbox = bbox;
		}
	}

	return qd;
}


QuadricData QuadricDataFactory::createDisk(float innerRadius, float outerRadius, float numslices, float numstacks, float z, float phimax) {
	QuadricData qd;
	ss::BoundingBox bbox;
	for (uint32_t i = 0; i < numslices; i++) {
		for (uint32_t j = 0; j < numstacks; j++) {
			float u = float(i) / float(numslices - 1);
			float v = float(j) / float(numstacks - 1);

			float phi = u * phimax;

			float x = ((1 - v) * innerRadius + v * outerRadius) * cosf(phi);
			float y = ((1 - v) * innerRadius + v * outerRadius) * sinf(phi);
			
			glm::vec4 position(x, y, z, 1.0f);
			glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
			glm::vec4 color(u, v, 0.5f, 1.0f);
			glm::vec2 texcoord(u, v);
			bbox.expandBy(position);

			if (i < (numslices - 1) && j < (numstacks - 1)) {
				uint32_t start = i * numstacks + j;

				uint32_t idx0 = start;
				uint32_t idx1 = start + numstacks;
				uint32_t idx2 = start + 1;

				uint32_t idx3 = start + 1;
				uint32_t idx4 = start + numstacks;
				uint32_t idx5 = start + numstacks + 1;

				qd.indices.push_back(idx0);
				qd.indices.push_back(idx1);
				qd.indices.push_back(idx2);
				qd.indices.push_back(idx3);
				qd.indices.push_back(idx4);
				qd.indices.push_back(idx5);
			}

			qd.bbox = bbox;
		}
	}
	return qd;
}

QuadricData QuadricDataFactory::createCone(float radius, float numslices, float numstacks, float height, float phimax) {
	QuadricData qd;
	ss::BoundingBox bbox;
	for (uint32_t i = 0; i < numslices; i++) {
		for (uint32_t j = 0; j < numstacks; j++) {
			float u = float(i) / float(numslices - 1);
			float v = float(j) / float(numstacks - 1);

			float phi = u * phimax;

			float x = radius * (1 - v) * cosf(phi);
			float y = radius * (1 - v) * sinf(phi);
			float z = v * height;

			glm::vec4 position(x, y, z, 1.0f);
			glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
			glm::vec4 color(u, v, 0.5f, 1.0f);
			glm::vec2 texcoord(u, v);
			bbox.expandBy(position);

			if (i < (numslices - 1) && j < (numstacks - 1)) {
				uint32_t start = i * numstacks + j;

				uint32_t idx0 = start;
				uint32_t idx1 = start + numstacks;
				uint32_t idx2 = start + 1;

				uint32_t idx3 = start + 1;
				uint32_t idx4 = start + numstacks;
				uint32_t idx5 = start + numstacks + 1;

				qd.indices.push_back(idx0);
				qd.indices.push_back(idx1);
				qd.indices.push_back(idx2);
				qd.indices.push_back(idx3);
				qd.indices.push_back(idx4);
				qd.indices.push_back(idx5);
			}

			qd.bbox = bbox;
		}
	}

	return qd;
}