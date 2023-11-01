#include "QuadricDataFactory.h"
#include <math.h>
#include <VkRenderSystem.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ss {
	namespace data {
		const float QuadricDataFactory::RS_PI = 3.14159265358979323846f;
		const float QuadricDataFactory::DEFAULT_NUM_STACKS = 10.0f;
		const float QuadricDataFactory::DEFAULT_NUM_SLICES = 10.0f;
		const float QuadricDataFactory::DEFAULT_RADIUS = 0.5f;
		const float QuadricDataFactory::DEFAULT_PHI_MAX = 2 * RS_PI;
		const float QuadricDataFactory::DEFAULT_SIZE = 0.5f;
		const float QuadricDataFactory::DEFAULT_HEIGHT = 0.5;
		const float QuadricDataFactory::DEFAULT_HALF_RADIUS = QuadricDataFactory::DEFAULT_RADIUS * 0.5f;


		void QuadricDataFactory::initRSgeometry(MeshData& md)
		{
			VkRenderSystem& vkrs = VkRenderSystem::getInstance();

			md.bbox = ss::BoundingBox(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

			//Create all vertex attribute format.
			std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
			RSvertexAttribsInfo attribInfo;
			attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
			attribInfo.attributes = attribs.data();
			attribInfo.settings = RSvertexAttributeSettings::vasSeparate;

			//Create geometry data
			uint32_t numVertices = static_cast<uint32_t>(md.positions.size());
			uint32_t numIndices = static_cast<uint32_t>(md.indices.size());

			vkrs.geometryDataCreate(md.geometryDataID, numVertices, numIndices, attribInfo);
			uint32_t posSizeInBytes = numVertices * sizeof(md.positions[0]);
			vkrs.geometryDataUpdateVertices(md.geometryDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)md.positions.data());

			uint32_t normSizeInBytes = numVertices * sizeof(md.normals[0]);
			vkrs.geometryDataUpdateVertices(md.geometryDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)md.normals.data());

			uint32_t colorSizeInBytes = numVertices * sizeof(md.colors[0]);
			vkrs.geometryDataUpdateVertices(md.geometryDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)md.colors.data());

			uint32_t texcoordSizeInBytes = numVertices * sizeof(md.texcoords[0]);
			vkrs.geometryDataUpdateVertices(md.geometryDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)md.texcoords.data());

			uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
			vkrs.geometryDataUpdateIndices(md.geometryDataID, 0, indicesSizeInBytes, (void*)md.indices.data());
			vkrs.geometryDataFinalize(md.geometryDataID);

			//Create geometry
			RSgeometryInfo geometry;
			geometry.primType = RSprimitiveType::ptTriangle;
			vkrs.geometryCreate(md.geometryID, geometry);
		}

		void QuadricDataFactory::initRSview(MeshData& md, MeshInstance& mi, const RScollectionID& collectionID, RStextureID textureID, RSshaderTemplate shaderTemplate, glm::mat4 xform)
		{
			VkRenderSystem& vkrs = VkRenderSystem::getInstance();
			RSappearanceInfo appInfo;
			appInfo.diffuseTexture = textureID;
			appInfo.shaderTemplate = shaderTemplate;

			vkrs.appearanceCreate(mi.appearanceID, appInfo);

			RSspatial spl;
			spl.model = xform;
			spl.modelInv = glm::inverse(spl.model);
			vkrs.spatialCreate(mi.spatialID, spl);

			RSinstanceInfo instInfo;
			instInfo.gdataID = md.geometryDataID;
			instInfo.geomID = md.geometryID;
			instInfo.appID = mi.appearanceID;
			instInfo.spatialID = mi.spatialID;

			vkrs.collectionInstanceCreate(collectionID, mi.instanceID, instInfo);
		}

		MeshData QuadricDataFactory::createSphere(float radius, uint32_t numslices, uint32_t numstacks)
		{
			MeshData qd;
			// theta -> [0, pi] vertical plane
			// phi -> [0, 2pi] horizontal plane
			float phiMax = 2 * RS_PI;
			float thetaMin = 0.0f;
			float thetaMax = RS_PI;
			ss::BoundingBox bbox;
			for (uint32_t i = 0; i < numslices; i++)
			{
				for (uint32_t j = 0; j < numstacks; j++)
				{
					float u = float(i) / float(numslices - 1);
					float v = float(j) / float(numstacks - 1);

					float phi = u * phiMax;
					float theta = thetaMin + v * (thetaMax - thetaMin);

					float x = radius * sinf(theta) * cosf(phi);
					float y = radius * sinf(theta) * sinf(phi);
					float z = radius * cosf(theta);

					glm::vec4 position(x, y, z, 1.0f);
					glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
					glm::vec4 color(u, v, 0.5f, 1.0f);
					glm::vec2 texcoord(u, v);

					qd.bbox.expandBy(position);

					qd.positions.push_back(position);
					qd.colors.push_back(color);
					qd.normals.push_back(normal);
					qd.texcoords.push_back(texcoord);

					if (i < (numslices - 1) && j < (numstacks - 1))
					{
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
				}
			}

			return qd;
		}

		MeshData QuadricDataFactory::createQuad(float size)
		{

			MeshData qd;
			float halfsize = size / 2.f;
			qd.positions = std::vector<glm::vec4>{
				{-halfsize, -halfsize, 0.0f, 1.0f},
				{halfsize, -halfsize, 0.0f, 1.0f},
				{halfsize, halfsize, 0.0f, 1.0f},
				{-halfsize, halfsize, 0.0f, 1.0f} };

			qd.normals = std::vector<glm::vec4>{
				{1.0f, 0.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f, 1.0f} };

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
				{1.0f, 1.0f} };

			qd.bbox = ss::BoundingBox(glm::vec4(-halfsize, -halfsize, 0, 1), glm::vec4(halfsize, halfsize, 0, 1));

			qd.indices = {
				0, 1, 2, 2, 3, 0 };

			return qd;
		}

		MeshData QuadricDataFactory::createCylinder(float radius, uint32_t numslices, uint32_t numstacks, float zmin, float zmax, float phimax)
		{
			MeshData qd;
			float phiMax = phimax;
			ss::BoundingBox bbox;
			for (uint32_t i = 0; i < numslices; i++)
			{
				for (uint32_t j = 0; j < numstacks; j++)
				{
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
					qd.bbox.expandBy(position);

					if (i < (numslices - 1) && j < (numstacks - 1))
					{
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
				}
			}

			return qd;
		}

		MeshData QuadricDataFactory::createDisk(float innerRadius, float outerRadius, uint32_t numslices, uint32_t numstacks, float z, float phimax)
		{
			MeshData qd;
			ss::BoundingBox bbox;
			for (uint32_t i = 0; i < numslices; i++)
			{
				for (uint32_t j = 0; j < numstacks; j++)
				{
					float u = float(i) / float(numslices - 1);
					float v = float(j) / float(numstacks - 1);

					float phi = u * phimax;

					float x = ((1 - v) * innerRadius + v * outerRadius) * cosf(phi);
					float y = ((1 - v) * innerRadius + v * outerRadius) * sinf(phi);

					glm::vec4 position(x, y, z, 1.0f);
					glm::vec4 normal = glm::normalize(glm::vec4(position.x, position.y, position.z, 0.0f));
					glm::vec4 color(u, v, 0.5f, 1.0f);
					glm::vec2 texcoord(u, v);
					qd.bbox.expandBy(position);

					qd.positions.push_back(position);
					qd.colors.push_back(color);
					qd.normals.push_back(normal);
					qd.texcoords.push_back(texcoord);

					if (i < (numslices - 1) && j < (numstacks - 1))
					{
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
				}
			}
			return qd;
		}

		MeshData QuadricDataFactory::createCone(float radius, uint32_t numslices, uint32_t numstacks, float height, float phimax)
		{
			MeshData qd;
			ss::BoundingBox bbox;
			for (uint32_t i = 0; i < numslices; i++)
			{
				for (uint32_t j = 0; j < numstacks; j++)
				{
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
					qd.bbox.expandBy(position);

					qd.positions.push_back(position);
					qd.colors.push_back(color);
					qd.normals.push_back(normal);
					qd.texcoords.push_back(texcoord);

					if (i < (numslices - 1) && j < (numstacks - 1))
					{
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
				}
			}

			return qd;
		}
	}
}
