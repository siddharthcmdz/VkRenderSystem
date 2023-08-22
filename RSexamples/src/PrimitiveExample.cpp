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

	case PrimitiveType::Solid:
		geomInfo.primType = RSprimitiveType::ptTriangle;
		break;
	}
	vkrs.geometryCreate(ce.geomID, geomInfo);

	RScollectionInfo collInfo;
	vkrs.collectionCreate(ce.collectionID, collInfo);
	vkrs.viewAddCollection(iviewID, ce.collectionID);

	RSinstanceInfo instInfo;
	instInfo.gdataID = ce.geomDataID;
	instInfo.geomID = ce.geomID;

	if (pt == PrimitiveType::Lines) {
		RSstate state;
		state.lnstate.lineWidth = 7.0f;
		vkrs.stateCreate(ce.stateID, state);
		instInfo.stateID = ce.stateID;
	}

	RSappearanceInfo appInfo;
	appInfo.shaderTemplate = RSshaderTemplate::stPassthrough;
	vkrs.appearanceCreate(ce.appID, appInfo);
	instInfo.appID = ce.appID;
	RSspatial spl;
	spl.model = glm::scale(glm::mat4(1), glm::vec3(2.0f, 2.0f, 2.0f));
	spl.modelInv = glm::inverse(spl.model);
	vkrs.spatialCreate(ce.spatialID, spl);
	instInfo.spatialID = ce.spatialID;

	vkrs.collectionInstanceCreate(ce.collectionID, ce.instanceID, instInfo);
	vkrs.collectionFinalize(ce.collectionID, ictxID, iviewID);

}
std::vector<rsvd::VertexPC> PrimitiveExample::getVertices(PrimitiveType pt, float radius) {
	//create the geometry data
	std::vector<rsvd::VertexPC> vertexDataList;
	uint32_t numSamples = 6;
	glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	for (uint32_t i = 0; i < numSamples; i++) {
		//float theta = (float(i + 1) / float(numSamples)) * (float)RS_PI;
		float theta = (2 * (float)RS_PI * i) / numSamples;
		float x = radius * cos(theta);
		x = abs(x) < 1e-2 ? 0.0f : x;
		float y = radius * sin(theta);
		y = abs(y) < 1e-2 ? 0.0f : y;
		rsvd::VertexPC vert;
		vert.pos = glm::vec4(x, y, 0.0, 1.0f);
		std::cout << "theta: " << theta * (180 / RS_PI) << ", (" << x << ", " << y << ")" << std::endl;
		if (pt == PrimitiveType::Solid) {
			float t = float(i + 1) / float(numSamples);
			vert.color = glm::mix(red, green, t);
		}
		else if (pt == PrimitiveType::Lines) {
			vert.color = glm::vec4(0, 0, 1, 1);
		}
		else {
			vert.color = glm::vec4(0, 0, 0, 1);
		}

		vertexDataList.push_back(vert);
	}

	return vertexDataList;
}

void PrimitiveExample::init() {


	int pointidx = PrimitiveType::Points;
	int lineidx = PrimitiveType::Lines;
	int solididx = PrimitiveType::Solid;
	float radius = 0.5f;
	icircles[pointidx].vertices = getVertices(PrimitiveType::Points, radius);
	icircles[pointidx].radius = 0.5;
	icircles[lineidx].vertices = getVertices(PrimitiveType::Lines, radius);
	icircles[lineidx].vertices.push_back(icircles[lineidx].vertices[0]);
	icircles[lineidx].radius = radius;

	std::vector<rsvd::VertexPC> solidVerts = getVertices(PrimitiveType::Solid, radius);
	for (uint32_t i = 0; i < solidVerts.size() - 2; i++) {
		rsvd::VertexPC vert0, vert1, vert2;
		vert0 = solidVerts[0];
		vert1 = solidVerts[i+1];
		vert2 = solidVerts[i+2];
		icircles[solididx].vertices.push_back(vert0);
		icircles[solididx].vertices.push_back(vert1);
		icircles[solididx].vertices.push_back(vert2);
	}
	icircles[solididx].radius = radius;

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
	sprintf_s(ctxInfo.title, "Primitive example");
	vkrs.contextCreate(ictxID, ctxInfo);

	RSview rsview;
	rsview.cameraType = CameraType::ORBITAL;
	rsview.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vkrs.viewCreate(iviewID, rsview);

	createEntity(icircles[PrimitiveType::Points], PrimitiveType::Points);
	createEntity(icircles[PrimitiveType::Lines], PrimitiveType::Lines);
	createEntity(icircles[PrimitiveType::Solid], PrimitiveType::Solid);
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
	vkrs.stateDispose(this->stateID);
	vkrs.collectionInstanceDispose(this->collectionID, this->instanceID);
	vkrs.collectionDispose(this->collectionID);
}