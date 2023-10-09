#include "ModelData.h"
#include <VkRenderSystem.h>

namespace ss {
	void* MeshData::getAttribData(const RSvertexAttribute attrib) {
		switch (attrib) {
		case RSvertexAttribute::vaPosition:
			return positions.data();

		case RSvertexAttribute::vaNormal:
			return normals.data();

		case RSvertexAttribute::vaColor:
			return colors.data();

		case RSvertexAttribute::vaTexCoord:
			return texcoords.data();
		}

		return nullptr;
	}

	void MeshData::dispose() {
		auto& vkrs = VkRenderSystem::getInstance();
		vkrs.geometryDataDispose(this->geometryDataID);
		vkrs.geometryDispose(this->geometryID);
		delete[] this->attribsInfo.attributes;
		this->attribsInfo.attributes = nullptr;
	}

	void Appearance::dispose() {
		auto& vkrs = VkRenderSystem::getInstance();
	}

	void MeshInstance::dispose() {
		auto& vkrs = VkRenderSystem::getInstance();
		vkrs.collectionInstanceDispose(this->associatedCollectionID, this->instanceID);
		vkrs.textureDispose(this->diffuseTextureID);
		vkrs.appearanceDispose(this->appearanceID);
		meshData.dispose();
	}
}