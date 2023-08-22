#include "PrimitiveExample.h"
#include <cstdint>

#include <cmath>
#include <iostream>

#define RS_PI 3.14159265358979323846
#include <VkRenderSystem.h>
#include <glm/gtc/matrix_transform.hpp>
#include "helper.h"

PrimitiveExample::PrimitiveExample() {

}

void PrimitiveExample::initShaderPath(RSinitInfo& initInfo) {
	std::string currDir = helper::getCurrentDir();
	std::cout << "current dir: " << currDir << std::endl;
	std::string shaderDir = currDir + "\\shaders";
	strcpy_s(initInfo.shaderPath, shaderDir.c_str());
}

void PrimitiveExample::createEntity(CircleEntity& ce, PrimitiveType pt) {
	auto& vkrs = VkRenderSystem::getInstance();

	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor};
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	vkrs.geometryDataCreate(ce.geomDataID, static_cast<uint32_t>(ce.vertices.size()), 0, attribInfo, RSbufferUsageHints::buVertices);
	uint32_t vertSizeInBytes = static_cast<uint32_t>(ce.vertices.size() * sizeof(ce.vertices[0]));
	vkrs.geometryDataUpdateVertices(ce.geomDataID, 0, vertSizeInBytes, (void*)ce.vertices.data());
	uint32_t indicesSizeInBytes = 0;
	vkrs.geometryDataFinalize(ce.geomDataID);

	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptPoint;
	switch (pt) {
	case PrimitiveType::Points:
		geomInfo.primType = RSprimitiveType::ptPoint;
		break;

	case PrimitiveType::Lines:
		geomInfo.primType = RSprimitiveType::ptLineStrip;
		break;
	}
	vkrs.geometryCreate(ce.geomID, geomInfo);

	RScollectionInfo collInfo;
	vkrs.collectionCreate(ce.collectionID, collInfo);
	vkrs.viewAddCollection(iviewID, ce.collectionID);

	RSinstanceInfo instInfo;
	instInfo.gdataID = ce.geomDataID;
	instInfo.geomID = ce.geomID;

	RSappearanceInfo appInfo;
	appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
	vkrs.appearanceCreate(ce.appID, appInfo);
	instInfo.appID = ce.appID;
	RSspatial spl;
	spl.model = glm::mat4(1);//glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
	spl.modelInv = glm::inverse(spl.model);
	vkrs.spatialCreate(ce.spatialID, spl);
	instInfo.spatialID = ce.spatialID;

	vkrs.collectionInstanceCreate(ce.collectionID, ce.instanceID, instInfo);
	vkrs.collectionFinalize(ce.collectionID, ictxID, iviewID);

}

void PrimitiveExample::init() {

	//create the geometry data
	std::vector<rsvd::VertexPC> vertexDataList;
	float radius = 0.5f;
	uint32_t numSamples = 4;
	glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	for (uint32_t i = 0; i < numSamples; i++) {
		float theta = (float(i + 1) / float(numSamples)) * (float)RS_PI;
		float x = radius * cos(theta);
		float y = radius * sin(theta);
		rsvd::VertexPC vert;
		vert.pos = glm::vec4(x, y, 0.0, 1.0f);
		float t = float(i + 1) / float(numSamples);
		vert.color = glm::mix(red, green, t);

		vertexDataList.push_back(vert);
	}

	icircles[POINT_CIRCLE].vertices = vertexDataList;
	icircles[POINT_CIRCLE].radius = radius;

	icircles[LINE_CIRCLE].vertices = vertexDataList;
	icircles[LINE_CIRCLE].radius = radius;


	auto& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "PrimitiveExample");
	info.enableValidation = true;
	info.onScreenCanvas = true;
	initShaderPath(info);

	vkrs.renderSystemInit(info);
	

	RScontextInfo ctxInfo;
	ctxInfo.width = 800;
	ctxInfo.height = 600;
	sprintf_s(ctxInfo.title, "Hello Triangle");
	vkrs.contextCreate(ictxID, ctxInfo);

	RSview rsview;
	rsview.cameraType = CameraType::ORBITAL;
	rsview.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vkrs.viewCreate(iviewID, rsview);

	createEntity(icircles[PrimitiveType::Points], PrimitiveType::Points);
	createEntity(icircles[PrimitiveType::Lines], PrimitiveType::Lines);
}

void PrimitiveExample::render() {
	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(ictxID, iviewID);
}

void PrimitiveExample::dispose() {
	auto& vkrs = VkRenderSystem::getInstance();
	for (auto& ent : icircles) {
		ent.dispose();
	}
	vkrs.contextDispose(ictxID);
	vkrs.viewDispose(iviewID);

	vkrs.renderSystemDispose();
}


void CircleEntity::dispose() {
	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.geometryDataDispose(this->geomDataID);
	vkrs.appearanceDispose(this->appID);
	vkrs.spatialDispose(this->spatialID);
	vkrs.collectionInstanceDispose(this->collectionID, this->instanceID);
	vkrs.collectionDispose(this->collectionID);
}