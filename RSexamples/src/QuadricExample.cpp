#include "QuadricExample.h"
#include "QuadricDataFactory.h"
#include <VkRenderSystem.h>
#include <glm/gtc/matrix_transform.hpp>

QuadricExample::QuadricExample() {

}

void QuadricExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	QuadricData sphereData = QuadricDataFactory::createSphere(1, 6, 6);
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();

	ibbox = ss::BoundingBox(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaNormal, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	attribInfo.settings = RSvertexAttributeSettings::vasSeparate;
	
	RSsingleEntity entity;
	vkrs.geometryDataCreate(entity.geomDataID, static_cast<uint32_t>(sphereData.positions.size()), static_cast<uint32_t>(sphereData.indices.size()), attribInfo);
	uint32_t posSizeInBytes = static_cast<uint32_t>(sphereData.positions.size() * sizeof(sphereData.positions[0]));
	vkrs.geometryDataUpdateVertices(entity.geomDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)sphereData.positions.data());

	uint32_t normSizeInBytes = static_cast<uint32_t>(sphereData.normals.size()) * sizeof(sphereData.normals[0]);
	vkrs.geometryDataUpdateVertices(entity.geomDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)sphereData.normals.data());

	uint32_t colorSizeInBytes = static_cast<uint32_t>(sphereData.colors.size()) * sizeof(sphereData.colors[0]);
	vkrs.geometryDataUpdateVertices(entity.geomDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)sphereData.colors.data());

	uint32_t texcoordSizeInBytes = static_cast<uint32_t>(sphereData.texcoords.size()) * sizeof(sphereData.texcoords[0]);
	vkrs.geometryDataUpdateVertices(entity.geomDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)sphereData.texcoords.data());

	uint32_t indicesSizeInBytes = static_cast<uint32_t>(sphereData.indices.size()) * sizeof(uint32_t);
	vkrs.geometryDataUpdateIndices(entity.geomDataID, 0, indicesSizeInBytes, (void*)sphereData.indices.data());
	vkrs.geometryDataFinalize(entity.geomDataID);

	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptTriangle;
	vkrs.geometryCreate(entity.geomID, geomInfo);

	RScollectionInfo collInfo;
	vkrs.collectionCreate(entity.collectionID, collInfo);
	vkrs.viewAddCollection(globals.viewID, entity.collectionID);
	RSinstanceInfo instInfo;
	instInfo.gdataID = entity.geomDataID;
	instInfo.geomID = entity.geomID;


	RSappearanceInfo appInfo;
	appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
	vkrs.appearanceCreate(entity.appID, appInfo);
	instInfo.appID = entity.appID;
	RSspatial spl;
	spl.model = glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
	spl.modelInv = glm::inverse(spl.model);
	vkrs.spatialCreate(entity.spatialID, spl);
	instInfo.spatialID = entity.spatialID;

	vkrs.collectionInstanceCreate(entity.collectionID, entity.instanceID, instInfo);
	vkrs.collectionFinalize(entity.collectionID, globals.ctxID, globals.viewID);

	ibbox.expandBy(sphereData.bbox.getmin());
	ibbox.expandBy(sphereData.bbox.getmax());

	iquadrics.push_back(entity);
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
