#include "QuadricExample.h"
#include "QuadricDataFactory.h"
#include <VkRenderSystem.h>
#include <glm/gtc/matrix_transform.hpp>

QuadricExample::QuadricExample() {

}

void QuadricExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	std::vector<QuadricData> qdatalist = {
		QuadricDataFactory::createSphere(),
		QuadricDataFactory::createCone(),
		QuadricDataFactory::createCylinder(),
		QuadricDataFactory::createDisk(),
		QuadricDataFactory::createQuad()
	};

	VkRenderSystem& vkrs = VkRenderSystem::getInstance();

	ibbox = ss::BoundingBox(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
	
	RScollectionInfo collInfo;
	vkrs.collectionCreate(icollectionID, collInfo);
	vkrs.viewAddCollection(globals.viewID, icollectionID);
	float start = -(qdatalist.size() * 0.5f);
	for (size_t i = 0; i < qdatalist.size(); i++) {
		RSsingleEntity se;

		uint32_t numVertices = static_cast<uint32_t>(qdatalist[i].positions.size());
		uint32_t numIndices = static_cast<uint32_t>(qdatalist[i].indices.size());

		vkrs.geometryDataCreate(se.geomDataID, numVertices, numIndices, attribInfo);
		uint32_t posSizeInBytes = numVertices * sizeof(qdatalist[i].positions[0]);
		vkrs.geometryDataUpdateVertices(se.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)qdatalist[i].positions.data());

		uint32_t normSizeInBytes = numVertices * sizeof(qdatalist[i].normals[0]);
		vkrs.geometryDataUpdateVertices(se.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)qdatalist[i].normals.data());

		uint32_t colorSizeInBytes = numVertices * sizeof(qdatalist[i].colors[0]);
		vkrs.geometryDataUpdateVertices(se.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)qdatalist[i].colors.data());

		uint32_t texcoordSizeInBytes = numVertices * sizeof(qdatalist[i].texcoords[0]);
		vkrs.geometryDataUpdateVertices(se.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)qdatalist[i].texcoords.data());

		uint32_t indicesSizeInBytes = numIndices * sizeof(uint32_t);
		vkrs.geometryDataUpdateIndices(se.geomDataID, 0, indicesSizeInBytes, (void*)qdatalist[i].indices.data());
		vkrs.geometryDataFinalize(se.geomDataID);

		RSgeometryInfo geometry;
		geometry.primType = RSprimitiveType::ptTriangle;
		vkrs.geometryCreate(se.geomID, geometry);

		RSappearanceInfo appInfo;
		appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
		vkrs.appearanceCreate(se.appID, appInfo);

		RSspatial spl;
		

		spl.model = glm::translate(glm::mat4(1), glm::vec3(start, 0.0f, 0.0f));
		spl.modelInv = glm::inverse(spl.model);
		vkrs.spatialCreate(se.spatialID, spl);
		start += 1.0f;

		RSinstanceInfo instInfo;
		instInfo.gdataID = se.geomDataID;
		instInfo.geomID = se.geomID;
		instInfo.appID = se.appID;
		instInfo.spatialID = se.spatialID;

		vkrs.collectionInstanceCreate(icollectionID, se.instanceID, instInfo);

		glm::vec4 minpt = qdatalist[i].bbox.getmin();
		glm::vec4 maxpt = qdatalist[i].bbox.getmax();
		ibbox.expandBy(minpt);
		ibbox.expandBy(maxpt);

		iquadrics.push_back(se);

	}

	vkrs.collectionFinalize(icollectionID, globals.ctxID, globals.viewID);
}

void QuadricExample::render(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(globals.ctxID, globals.viewID);

}

void QuadricExample::dispose(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();
	for (RSsingleEntity& entity : iquadrics) {
		vkrs.geometryDataDispose(entity.geomDataID);
		vkrs.appearanceDispose(entity.appID);
		vkrs.spatialDispose(entity.spatialID);
		if (entity.stateID.isValid()) {
			vkrs.stateDispose(entity.stateID);
		}
		vkrs.collectionInstanceDispose(entity.collectionID, entity.instanceID);
		vkrs.collectionDispose(entity.collectionID);
	}
}

ss::BoundingBox QuadricExample::getBounds() {
	return ibbox;
}

std::string QuadricExample::getExampleName() const {
	return "QuadricExample";
}
