#include "PrimitiveExample.h"
#include <cstdint>

#include <cmath>
#include <iostream>

#define RS_PI 3.14159265358979323846
#include <VkRenderSystem.h>
#include <Windows.h>

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
	
	
}

void PrimitiveExample::render() {

}

void PrimitiveExample::dispose() {

}
