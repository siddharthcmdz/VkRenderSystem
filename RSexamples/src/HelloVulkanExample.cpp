#include "HelloVulkanExample.h"
#include <rsids.h>
#include <VkRenderSystem.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

HelloVulkanExample::HelloVulkanExample() {}

void HelloVulkanExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();

	ibbox = BoundingBox(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	attribInfo.settings = RSvertexAttributeSettings::vasInterleaved;
	vkrs.geometryDataCreate(ientity.geomDataID, static_cast<uint32_t>(ivertices.size()), static_cast<uint32_t>(iindices.size()), attribInfo);
	uint32_t vertSizeInBytes = static_cast<uint32_t>(ivertices.size() * sizeof(ivertices[0]));
	vkrs.geometryDataUpdateInterleavedVertices(ientity.geomDataID, 0, vertSizeInBytes, (void*)ivertices.data());
	uint32_t indicesSizeInBytes = static_cast<uint32_t>(iindices.size()) * sizeof(uint32_t);
	vkrs.geometryDataUpdateIndices(ientity.geomDataID, 0, indicesSizeInBytes, (void*)iindices.data());
	vkrs.geometryDataFinalize(ientity.geomDataID);

	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptTriangle;
	vkrs.geometryCreate(ientity.geomID, geomInfo);

	RScollectionInfo collInfo;
	vkrs.collectionCreate(ientity.collectionID, collInfo);
	vkrs.viewAddCollection(globals.viewID, ientity.collectionID);
	RSinstanceInfo instInfo;
	instInfo.gdataID = ientity.geomDataID;
	instInfo.geomID = ientity.geomID;

	vkrs.textureCreate(ientity.textureID, "C:\\Projects\\FSI\\RSexamples\\i386\\x64\\Debug\\textures\\texture.jpg");

	RSappearanceInfo appInfo;
	appInfo.diffuseTexture = ientity.textureID;
	appInfo.shaderTemplate = RSshaderTemplate::stTextured;
	vkrs.appearanceCreate(ientity.appID, appInfo);
	instInfo.appID = ientity.appID;
	RSspatial spl;
	spl.model = glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
	spl.modelInv = glm::inverse(spl.model);
	vkrs.spatialCreate(ientity.spatialID, spl);
	instInfo.spatialID = ientity.spatialID;

	vkrs.collectionInstanceCreate(ientity.collectionID, ientity.instanceID, instInfo);
	vkrs.collectionFinalize(ientity.collectionID, globals.ctxID, globals.viewID);
}

void HelloVulkanExample::render(const RSexampleGlobal& globals) {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(globals.ctxID, globals.viewID);
}

void HelloVulkanExample::dispose(const RSexampleGlobal& globals) {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	vkrs.geometryDataDispose(ientity.geomDataID);
	vkrs.textureDispose(ientity.textureID);
	vkrs.appearanceDispose(ientity.appID);
	vkrs.spatialDispose(ientity.spatialID);
	vkrs.collectionInstanceDispose(ientity.collectionID, ientity.instanceID);
	vkrs.collectionDispose(ientity.collectionID);
}

BoundingBox HelloVulkanExample::getBounds() {
	return ibbox;
}

std::string HelloVulkanExample::getExampleName() const {
	return "HelloVulkanExample";
}
