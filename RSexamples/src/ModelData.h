#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "BoundingBox.h"
#include <rsids.h>
#include <RSdataTypes.h>

namespace ss {
	struct ModelCapabilities {
		bool hasMesh = false;
		bool hasMaterials = false;
		bool hasTextures = false;
		bool hasAnimations = false;
		bool hasSkeleton = false;
		bool hasLights = false;
		bool hasCameras = false;
	};

	enum IndicesIntType {
		iitUINT32,
		iitUINT16,
		iitUINT8,
		iintInvalid,
	};

	struct MeshData {
		std::vector<glm::vec4> positions;
		std::vector<glm::vec4> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> texcoords;
		std::vector<uint32_t> iindices;
		IndicesIntType indicesType = IndicesIntType::iitUINT32;

		RSgeometryDataID geometryDataID;
		RSgeometryID geometryID;
		RSvertexAttribsInfo attribsInfo;

		void* getAttribData(const RSvertexAttribute attrib);
		void dispose();
	};

	struct Appearance {
		glm::vec4 ambient = glm::vec4(1, 1, 1, 1);
		glm::vec4 diffuse = glm::vec4(0, 0, 0, 1);
		glm::vec4 specular = glm::vec4(0, 0, 0, 1);
		glm::vec4 emissive = glm::vec4(0, 0, 0, 1);
		float shininess = 0.0f;
		std::string diffuseTexturePath;
		bool isDiffuseTextureEmbedded = false;
		void dispose();
	};

	struct MeshInstance {
		MeshData meshData;
		glm::mat4 modelmat;

		RSinstanceID instanceID;
		RSspatialID spatialID;
		RSstateID stateID;
		RScollectionID associatedCollectionID; //do not dispose this
		RSappearanceID appearanceID;
		RStextureID diffuseTextureID;
		uint32_t materialIdx;

		void dispose();
	};

	enum TextureType {
		ttDiffuse,
		ttAmbient,
		ttSpecular
	};

	struct Texture {
		std::string texturePath;
		TextureType textureType;
	};

	struct ModelData {
		std::string modelName;
		BoundingBox bbox;
		ss::ModelCapabilities imdcaps;
		std::vector<MeshInstance> meshInstances;
		std::unordered_map<uint32_t, Appearance> materials;

		RScollectionID collectionID;
	};
}