#include "PrimitiveExample.h"
#include <cstdint>

#include <cmath>
#include <iostream>

#define RS_PI 3.14159265358979323846
#include <VkRenderSystem.h>
#include <Windows.h>
#include <glm/gtc/matrix_transform.hpp>

PrimitiveExample::PrimitiveExample() {

}

std::wstring getCurrentDir() {
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

void initShaderPath(RSinitInfo& initInfo) {
	std::wstring currDirW = getCurrentDir();
	std::string currDir(currDirW.begin(), currDirW.end());
	std::cout << "current dir: " << currDir << std::endl;
	currDir += "\\shaders";
	strcpy_s(initInfo.shaderPath, currDir.c_str());
}

void PrimitiveExample::init() {
	//create the geometry data
	float radius = 0.25f;
	uint32_t numSamples = 32;
	glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);
	for (uint32_t i = 0; i < numSamples; i++) {
		float theta = (float(i + 1) / float(numSamples)) * RS_PI;
		float x = radius * cos(theta);
		float y = radius * sin(theta);
		rsvd::VertexPC vert;
		vert.pos = glm::vec4(x, y, 0, 1);
		float t = (i + 1) / numSamples;
		vert.color = glm::mix(red, green, t);

		icirclePoints.push_back(vert);
	}

	icircleLineStrip = icirclePoints;
	icircleLineStrip.push_back(icircleLineStrip[0]);

	auto& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "PrimitiveExample");
	info.enableValidation = true;
	info.onScreenCanvas = true;
	initShaderPath(info);

	vkrs.renderSystemInit(info);
	
	RScontextID ctxID;
	RScontextInfo ctxInfo;
	ctxInfo.width = 800;
	ctxInfo.height = 600;
	sprintf_s(ctxInfo.title, "Hello Triangle");
	vkrs.contextCreate(ctxID, ctxInfo);

	RSviewID viewID;
	RSview rsview;
	rsview.clearColor = glm::vec4(0, 0, 0, 1);
	rsview.cameraType = CameraType::ORBITAL;
	rsview.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	vkrs.viewCreate(viewID, rsview);

	RSgeometryDataID gdataID;
	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor, RSvertexAttribute::vaTexCoord };
	RSvertexAttribsInfo attribInfo;
	attribInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribInfo.attributes = attribs.data();
	vkrs.geometryDataCreate(gdataID, static_cast<uint32_t>(ivertices.size()), static_cast<uint32_t>(iindices.size()), attribInfo, RSbufferUsageHints::buVertices);
	uint32_t vertSizeInBytes = static_cast<uint32_t>(ivertices.size() * sizeof(ivertices[0]));
	vkrs.geometryDataUpdateVertices(gdataID, 0, vertSizeInBytes, (void*)ivertices.data());
	uint32_t indicesSizeInBytes = static_cast<uint32_t>(iindices.size()) * sizeof(uint32_t);
	vkrs.geometryDataUpdateIndices(gdataID, 0, indicesSizeInBytes, (void*)iindices.data());
	vkrs.geometryDataFinalize(gdataID);

	RSgeometryID geomID;
	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptTriangle;
	vkrs.geometryCreate(geomID, geomInfo);

	RScollectionID collID;
	RScollectionInfo collInfo;
	vkrs.collectionCreate(collID, collInfo);
	vkrs.viewAddCollection(viewID, collID);
	RSinstanceID instID;
	RSinstanceInfo instInfo;
	instInfo.gdataID = gdataID;
	instInfo.geomID = geomID;

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

	vkrs.collectionInstanceCreate(collID, instID, instInfo);
	vkrs.collectionFinalize(collID, ctxID, viewID);

}

void PrimitiveExample::render() {
	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(ctxID, viewID);
}

void PrimitiveExample::dispose() {
	vkrs.geometryDataDispose(gdataID);
	vkrs.contextDispose(ctxID);
	vkrs.viewDispose(viewID);
	vkrs.textureDispose(texID);
	vkrs.appearanceDispose(appID);
	vkrs.spatialDispose(splID);
	vkrs.collectionInstanceDispose(collID, instID);
	vkrs.collectionDispose(collID);
	vkrs.renderSystemDispose();

}
