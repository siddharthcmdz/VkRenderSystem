#include "HelloVulkanExample.h"
#include <rsids.h>
#include <VkRenderSystem.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

HelloVulkanExample::HelloVulkanExample() {}

void getShaderPath(RSinitInfo& initInfo) {
	std::string currDir = helper::getCurrentDir();
	std::cout << "current dir: " << currDir << std::endl;
	currDir += "\\shaders";
	strcpy_s(initInfo.shaderPath, currDir.c_str());
}

void HelloVulkanExample::init() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "RenderSystem");
	info.enableValidation = true;
	info.onScreenCanvas = true;
	getShaderPath(info);
	vkrs.renderSystemInit(info);

	RScontextInfo ctxInfo;
	ctxInfo.width = 800;
	ctxInfo.height = 600;
	sprintf_s(ctxInfo.title, "Hello Triangle");
	vkrs.contextCreate(ictxID, ctxInfo);

	RSview rsview;
	rsview.clearColor = glm::vec4(0, 0, 0, 1);
	rsview.cameraType = CameraType::ORBITAL;
	rsview.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vkrs.viewCreate(iviewID, rsview);

	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	vkrs.geometryDataCreate(ientity.geomDataID, static_cast<uint32_t>(ivertices.size()), static_cast<uint32_t>(iindices.size()), attribInfo, RSbufferUsageHints::buVertices);
	uint32_t vertSizeInBytes = static_cast<uint32_t>(ivertices.size() * sizeof(ivertices[0]));
	vkrs.geometryDataUpdateVertices(ientity.geomDataID, 0, vertSizeInBytes, (void*)ivertices.data());
	uint32_t indicesSizeInBytes = static_cast<uint32_t>(iindices.size()) * sizeof(uint32_t);
	vkrs.geometryDataUpdateIndices(ientity.geomDataID, 0, indicesSizeInBytes, (void*)iindices.data());
	vkrs.geometryDataFinalize(ientity.geomDataID);

	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptTriangle;
	vkrs.geometryCreate(ientity.geomID, geomInfo);

	RScollectionInfo collInfo;
	vkrs.collectionCreate(ientity.collectionID, collInfo);
	vkrs.viewAddCollection(iviewID, ientity.collectionID);
	RSinstanceInfo instInfo;
	instInfo.gdataID = ientity.geomDataID;
	instInfo.geomID = ientity.geomID;

	RStextureID texID;
	vkrs.textureCreate(texID, "C:\\Projects\\FSI\\RSexamples\\i386\\x64\\Debug\\textures\\texture.jpg");

	RSappearanceID appID;
	RSappearanceInfo appInfo;
	appInfo.diffuseTexture = texID;
	appInfo.shaderTemplate = RSshaderTemplate::stTextured;
	vkrs.appearanceCreate(appID, appInfo);
	instInfo.appID = appID;
	RSspatialID splID;
	RSspatial spl;
	spl.model = glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
	spl.modelInv = glm::inverse(spl.model);
	vkrs.spatialCreate(splID, spl);
	instInfo.spatialID = splID;

	vkrs.collectionInstanceCreate(ientity.collectionID, ientity.instanceID, instInfo);
	vkrs.collectionFinalize(ientity.collectionID, ictxID, iviewID);
}

void HelloVulkanExample::render() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(ictxID, iviewID);
}

void HelloVulkanExample::dispose() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	vkrs.geometryDataDispose(ientity.geomDataID);
	vkrs.contextDispose(ictxID);
	vkrs.viewDispose(iviewID);
	vkrs.textureDispose(ientity.textureID);
	vkrs.appearanceDispose(ientity.appID);
	vkrs.spatialDispose(ientity.spatialID);
	vkrs.collectionInstanceDispose(ientity.collectionID, ientity.instanceID);
	vkrs.collectionDispose(ientity.collectionID);

	vkrs.renderSystemDispose();
}