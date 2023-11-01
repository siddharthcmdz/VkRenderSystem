#include "QuadricExample.h"
#include "QuadricDataFactory.h"
#include <VkRenderSystem.h>
#include <glm/gtc/matrix_transform.hpp>

QuadricExample::QuadricExample() {

}

void QuadricExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	std::vector<ss::MeshData> meshDataList = {
		ss::data::QuadricDataFactory::createSphere(),
		ss::data::QuadricDataFactory::createCone(),
		ss::data::QuadricDataFactory::createCylinder(),
		ss::data::QuadricDataFactory::createDisk(),
		ss::data::QuadricDataFactory::createQuad()
	};

	VkRenderSystem& vkrs = VkRenderSystem::getInstance();

	ibbox = ss::BoundingBox(glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	RScollectionInfo collInfo;
	collInfo.maxInstances = static_cast<uint32_t>(meshDataList.size());
	vkrs.collectionCreate(icollectionID, collInfo);
	vkrs.viewAddCollection(globals.viewID, icollectionID);
	
	float start = -(meshDataList.size() * 0.5f);
	for (size_t i = 0; i < meshDataList.size(); i++) {
		ss::MeshData md;
		ss::data::QuadricDataFactory::initRSgeometry(md);
		float dist = md.bbox.getmax().x - md.bbox.getmin().x;
		start += dist;
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(start, 0, 0));
		ss::MeshInstance mi;
		mi.meshData = md;
		mi.associatedCollectionID = icollectionID;
		ss::data::QuadricDataFactory::initRSview(md, mi, icollectionID, RStextureID(), RSshaderTemplate::stVolumeSlice, trans);
		
		glm::vec4 minpt = trans * md.bbox.getmin();
		glm::vec4 maxpt = trans * md.bbox.getmax();
		ibbox.expandBy(minpt);
		ibbox.expandBy(maxpt);

		imodelData.meshInstances.push_back(mi);
	}

	vkrs.collectionFinalize(icollectionID, globals.ctxID, globals.viewID);
}

void QuadricExample::render(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.contextDrawCollections(globals.ctxID, globals.viewID);

}

void QuadricExample::dispose(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();
	
	vkrs.collectionDispose(icollectionID);
	//TODO: dispose model data
}

ss::BoundingBox QuadricExample::getBounds() {
	return ibbox;
}

std::string QuadricExample::getExampleName() const {
	return "QuadricExample";
}
