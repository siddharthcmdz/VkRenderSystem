#include "QuadricDataFactory.h"
#include <math.h>

const float QuadricDataFactory::RS_PI = 3.14159265358979323846f;

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

			uint32_t idx0 = i;
			uint32_t idx1 = i + numstacks;
			uint32_t idx2 = i + j;
			uint32_t idx3 = i + j;
			uint32_t idx4 = i + numstacks;
			uint32_t idx5 = i + numstacks + 1;
			
			bbox.expandBy(position);

			qd.positions.push_back(position);
			qd.colors.push_back(color);
			qd.normals.push_back(normal);
			qd.texcoords.push_back(texcoord);
			qd.bbox = bbox;

			qd.indices.push_back(idx0);
			qd.indices.push_back(idx1);
			qd.indices.push_back(idx2);
			qd.indices.push_back(idx3);
			qd.indices.push_back(idx4);
			qd.indices.push_back(idx5);
		}
	}

	return qd;
}