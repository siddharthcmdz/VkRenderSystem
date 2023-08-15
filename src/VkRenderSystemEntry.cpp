#include "VkRenderSystemEntry.h"
#include "VkRenderSystem.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>           //for uint32_t
#include <limits>            //for std::numeric_limits
#include <algorithm>         //for std::clamp
#include <fstream>           //for file reading
#include "VertexData.h"


const std::vector<rsvd::VertexPC> ivertices = {
	{{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
	{{0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
	{{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> iindices = {
	0, 1, 2, 2, 3, 0
};


int RSmain() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "RenderSystem");
	info.enableValidation = true;
	info.onScreenCanvas = true;
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
	std::vector<RSvertexAttribute> attribs = { RSvertexAttribute::vaPosition, RSvertexAttribute::vaColor };
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
	vkrs.textureCreate(texID, "C:\\Projects\\VkRenderSystem\\x64\\Debug\\texture.jpg");

	RSappearanceID appID;
	RSappearanceInfo appInfo;
	appInfo.diffuseTexture = texID;
	vkrs.appearanceCreate(appID, appInfo);
	instInfo.appID = appID;
	//TODO: add spatialID to instinfo

	vkrs.collectionInstanceCreate(collID, instID, instInfo);
	vkrs.collectionFinalize(collID, ctxID, viewID);
	
	if (vkrs.isRenderSystemInit()) {
		vkrs.contextDrawCollections(ctxID, viewID);
	}

	vkrs.geometryDataDispose(gdataID);
	vkrs.contextDispose(ctxID);
	vkrs.viewDispose(viewID);
	vkrs.collectionInstanceDispose(collID, instID);
	vkrs.collectionDispose(collID);
	vkrs.renderSystemDispose();

	return 0;
}