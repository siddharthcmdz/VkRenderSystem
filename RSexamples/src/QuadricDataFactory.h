#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "ModelData.h"
#include "BoundingBox.h"

namespace ss {
	namespace data {
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
			static MeshData createSphere(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_STACKS, uint32_t numstacks = DEFAULT_NUM_SLICES);
			static MeshData createQuad(float size = DEFAULT_SIZE);
			static MeshData createCone(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float height = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
			static MeshData createCylinder(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float zmin = 0, float zmax = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
			static MeshData createDisk(float innerRadius = DEFAULT_HALF_RADIUS, float outerRadius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float z = 0, float phimax = DEFAULT_PHI_MAX);

			static void initRSgeometry(MeshData& md);
			static void initRSview(MeshData& md, MeshInstance& mi, const RScollectionID& collectionID, RStextureID textureID = RStextureID(), RSshaderTemplate shaderTemplate = RSshaderTemplate::stPassthrough, glm::mat4 xform = glm::mat4(1.0f));
		};
	}
}
