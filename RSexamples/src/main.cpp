
#include <iostream>
#include <conio.h>
#include <VkRenderSystem.h>
#include <VertexData.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdint>


const std::vector<rsvd::VertexPCT> ivertices = {
	{{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};
	
const std::vector<uint32_t> iindices = {
	0, 1, 2, 2, 3, 0
};

int main() {
	std::cout << "Hello world!" << std::endl;
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
	vkrs.textureCreate(texID, "C:\\Projects\\FSI\\RenderSystem\\src\\textures\\texture.jpg");

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
	
	vkrs.contextDrawCollections(ctxID, viewID);

	vkrs.geometryDataDispose(gdataID);
	vkrs.contextDispose(ctxID);
	vkrs.viewDispose(viewID);
	vkrs.textureDispose(texID);
	vkrs.appearanceDispose(appID);
	vkrs.spatialDispose(splID);
	vkrs.collectionInstanceDispose(collID, instID);
	vkrs.collectionDispose(collID);
	vkrs.renderSystemDispose();

	getchar();
	return 0;
}