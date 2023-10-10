#include "TinyGLTFexample.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ModelData.h"
#include <VkRenderSystem.h>

#pragma warning(disable : 4018)
#pragma warning(disable : 4267)
#pragma warning(error : 4996)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include "tiny_gltf.h"
#include <iostream>

static std::string indent(const int indent) {
	std::string s;
	for (int i = 0; i < indent; i++) {
		s += "  ";
	}
	return s;
}

static std::string printMode(int mode) {
	if (mode == TINYGLTF_MODE_POINTS) {
		return "POINTS";
	}
	else if (mode == TINYGLTF_MODE_LINE) {
		return "LINE";
	}
	else if (mode == TINYGLTF_MODE_LINE_LOOP) {
		return "LINE_LOOP";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLES) {
		return "TRIANGLES";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) {
		return "TRIANGLE_FAN";
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
		return "TRIANGLE_STRIP";
	}
	return "**UNKNOWN**";
}

static void DumpStringIntMap(const std::map<std::string, int>& m, int idnt) {
	std::map<std::string, int>::const_iterator it(m.begin());
	std::map<std::string, int>::const_iterator itEnd(m.end());
	for (; it != itEnd; it++) {
		std::cout << indent(idnt) << it->first << ": " << it->second << std::endl;
	}
}
static void dumpPrimitive(const tinygltf::Primitive& primitive, int idnt) {
	std::cout << indent(idnt) << "material : " << primitive.material << std::endl;
	std::cout << indent(idnt) << "indices : " << primitive.indices << std::endl;
	std::cout << indent(idnt) << "mode     : " << printMode(primitive.mode) << "(" << primitive.mode << ")" << std::endl;
	std::cout << indent(idnt) << "attributes(items=" << primitive.attributes.size() << ")" << std::endl;
	DumpStringIntMap(primitive.attributes, idnt + 1);
}

static void dump(const tinygltf::Model& model) {
	std::cout << "=== Dump glTF ===" << std::endl;
	std::cout << "asset.copyright          : " << model.asset.copyright << std::endl;
	std::cout << "asset.generator          : " << model.asset.generator << std::endl;
	std::cout << "asset.version            : " << model.asset.version << std::endl;
	std::cout << "asset.minVersion         : " << model.asset.minVersion << std::endl;
	std::cout << std::endl;

	std::cout << "=== Dump scene ===" << std::endl;
	std::cout << "defaultScene: " << model.defaultScene << std::endl;

	{
		std::cout << "meshes(item=" << model.meshes.size() << ")" << std::endl;
		for (size_t i = 0; i < model.meshes.size(); i++) {
			std::cout << indent(1) << "name     : " << model.meshes[i].name
				<< std::endl;
			std::cout << indent(1)
				<< "primitives(items=" << model.meshes[i].primitives.size()
				<< "): " << std::endl;

			for (size_t k = 0; k < model.meshes[i].primitives.size(); k++) {
				dumpPrimitive(model.meshes[i].primitives[k], 2);
			}
		}
	}
}

static std::vector<glm::vec4> transform3to4(const float* data, size_t numVertices) {
	std::vector<glm::vec4> attribs;
	attribs.resize(numVertices);
	size_t stride = 3;
	for (size_t i = 0; i < numVertices; i++) {
		size_t idx0 = i * stride + 0;
		size_t idx1 = i * stride + 1;
		size_t idx2 = i * stride + 2;

		float x = data[idx0];
		float y = data[idx1];
		float z = data[idx2];

		attribs[i] = glm::vec4(x, y, z, 1.0f);
	}

	return attribs;
}

static RSprimitiveType getPrimitiveMode(int mode) {
	if (mode == TINYGLTF_MODE_POINTS) {
		return RSprimitiveType::ptPoint;
	}
	else if (mode == TINYGLTF_MODE_LINE) {
		return RSprimitiveType::ptLine;
	}
	else if (mode == TINYGLTF_MODE_LINE_LOOP) {
		return RSprimitiveType::ptLineLoop;
	}
	else if (mode == TINYGLTF_MODE_TRIANGLES) {
		return RSprimitiveType::ptTriangle;
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_FAN) {
		return RSprimitiveType::ptTriangleFan;
	}
	else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
		return RSprimitiveType::ptTriangleStrip;
	}
	return RSprimitiveType::ptPoint;
}

static void populate(const tinygltf::Node& node, const tinygltf::Model& input, ss::ModelData& modelData) {
	glm::mat4 localMat(1.0f);
	
	if (node.translation.size() == 3) {
		localMat = glm::translate(localMat, glm::vec3(glm::make_vec3(node.translation.data())));
	}
	if (node.rotation.size() == 3) {
		glm::quat q = glm::make_quat(node.rotation.data());
		localMat *= glm::mat4(q);
	}
	if (node.scale.size() == 3) {
		localMat = glm::scale(localMat, glm::vec3(glm::make_vec3(node.scale.data())));
	}
	if (node.matrix.size() == 16) {
		localMat = glm::make_mat4x4(node.matrix.data());
	};

	//Load node's children
	for (size_t i = 0; i < node.children.size(); i++) {
		populate(input.nodes[node.children[i]], input, modelData);
	}

	auto& vkrs = VkRenderSystem::getInstance();
	//If node contains meshdata , load vertices and indices from buffers.
	//In gltf this is done via accessors and buffer views
	if (node.mesh > -1) {
		const tinygltf::Mesh mesh = input.meshes[node.mesh];
		ss::MeshInstance meshInst;
		ss::MeshData& meshData = meshInst.meshData;
		// Iterate through all primitives of this node's mesh
		for (size_t i = 0; i < mesh.primitives.size(); i++) {
			const tinygltf::Primitive& gltfPrimitive = mesh.primitives[i];
			std::vector<RSvertexAttribute> attribs;
			//Vertices 
			const float* positionBuffer = nullptr;
			const float* normalsBuffer = nullptr;
			const float* colorsBuffer = nullptr;
			const float* texCoordsBuffer = nullptr;
			size_t vertexCount = 0;
			// Get buffer data for vertex positions
			if (gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end()) {
				attribs.push_back(RSvertexAttribute::vaPosition);
				const tinygltf::Accessor& accessor = input.accessors[gltfPrimitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
				positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

				vertexCount = accessor.count;
				if (view.byteStride == 12) {
					meshData.positions = transform3to4(positionBuffer, vertexCount);
				}
			}
			// Get buffer data for vertex normals
			if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end()) {
				attribs.push_back(RSvertexAttribute::vaNormal);
				const tinygltf::Accessor& accessor = input.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
				normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

				if (view.byteStride == 12) {
					meshData.normals = transform3to4(normalsBuffer, vertexCount);
				}
			}
			//Get buffer data for color
			if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end()) {
				attribs.push_back(RSvertexAttribute::vaColor);
				const tinygltf::Accessor& accessor = input.accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
				const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
				colorsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));

				if (view.byteStride == 12) {
					meshData.colors = transform3to4(colorsBuffer, vertexCount);
				}
			}

			// Get buffer data for vertex texture coordinates
			// glTF supports multiple sets, we only load the first one
			if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end()) {
				attribs.push_back(RSvertexAttribute::vaTexCoord);
				const tinygltf::Accessor& accessor = input.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
				texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					
				meshData.texcoords.resize(vertexCount);
				for (size_t i = 0; i < vertexCount; i++) {
					meshData.texcoords[i].x = texCoordsBuffer[i * 2 + 0];
					meshData.texcoords[i].y = texCoordsBuffer[i * 2 + 1];
				}
			}

			RSvertexAttribsInfo attribsInfo;
			attribsInfo.attributes = attribs.data();
			attribsInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
			attribsInfo.settings = RSvertexAttributeSettings::vasSeparate;

			//Indices
			uint32_t indexCount = 0;
			const tinygltf::Accessor& accessor = input.accessors[gltfPrimitive.indices];
			const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

			indexCount += static_cast<uint32_t>(accessor.count);
			switch (accessor.componentType) {
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
				const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
					meshData.iindices.push_back(buf[index]);
				}
				meshData.indicesType = ss::IndicesIntType::iitUINT32;
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
				const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
					meshData.iindices.push_back(buf[index]);
				}
				meshData.indicesType = ss::IndicesIntType::iitUINT16;
				break;
			}
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
				const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
					meshData.iindices.push_back(buf[index]);
				}
				meshData.indicesType = ss::IndicesIntType::iitUINT8;
				break;
			}
			default:
				std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
				return;
			}

			vkrs.geometryDataCreate(meshData.geometryDataID, vertexCount, static_cast<uint32_t>(meshData.iindices.size()), attribsInfo);
			uint32_t posSizeInBytes = vertexCount * sizeof(meshData.positions[0]);
			vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, posSizeInBytes, RSvertexAttribute::vaPosition, (void*)meshData.positions.data());
			if (normalsBuffer != nullptr) {
				uint32_t normSizeInBytes = vertexCount * sizeof(meshData.normals[0]);
				vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, normSizeInBytes, RSvertexAttribute::vaNormal, (void*)meshData.normals.data());
			}
			if (colorsBuffer != nullptr) {
				uint32_t colorSizeInBytes = vertexCount * sizeof(meshData.colors[0]);
				vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, colorSizeInBytes, RSvertexAttribute::vaColor, (void*)meshData.colors.data());
			}
			if (texCoordsBuffer != nullptr) {
				uint32_t texcoordSizeInBytes = vertexCount * sizeof(meshData.texcoords[0]);
				vkrs.geometryDataUpdateVertices(meshData.geometryDataID, 0, texcoordSizeInBytes, RSvertexAttribute::vaTexCoord, (void*)meshData.texcoords.data());
			}

			if (meshData.iindices.size()) {
				uint32_t indicesSizeInBytes = indexCount * sizeof(uint32_t);
				vkrs.geometryDataUpdateIndices(meshData.geometryDataID, 0, indicesSizeInBytes, (void*)meshData.iindices.data());
			}
			vkrs.geometryDataFinalize(meshData.geometryDataID);

			RSgeometryInfo geomInfo;
			geomInfo.primType = getPrimitiveMode(gltfPrimitive.mode);
			vkrs.geometryCreate(meshData.geometryID, geomInfo);
			
			meshInst.materialIdx = gltfPrimitive.material;

			RSspatial spatial;
			spatial.model = localMat;
			spatial.modelInv = glm::inverse(spatial.model);

			vkrs.spatialCreate(meshInst.spatialID, spatial);
			RSappearanceInfo appInfo;
			appInfo.shaderTemplate = RSshaderTemplate::stSimpleLit;
			vkrs.appearanceCreate(meshInst.appearanceID, appInfo);

			RSinstanceInfo instInfo;
			instInfo.gdataID = meshData.geometryDataID;
			instInfo.geomID = meshData.geometryID;
			instInfo.spatialID = meshInst.spatialID;
			instInfo.appID = meshInst.appearanceID;

			vkrs.collectionInstanceCreate(modelData.collectionID, meshInst.instanceID, instInfo);
		}

		
	}
}

static ss::ModelData populate(const tinygltf::Model& model, const RSexampleGlobal& globals) {
	ss::ModelData modelData;
	auto& vkrs = VkRenderSystem::getInstance();
	RScollectionInfo collInfo;
	collInfo.maxInstances = 1000;
	vkrs.collectionCreate(modelData.collectionID, collInfo);

	const tinygltf::Scene& scene = model.scenes[0];
	for (size_t i = 0; i < scene.nodes.size(); i++) {
		const tinygltf::Node& node = model.nodes[scene.nodes[i]];
		populate(node, model, modelData);
	}

	vkrs.collectionFinalize(modelData.collectionID, globals.ctxID, globals.viewID);

	return modelData;
}

void TinyGLTFexample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	tinygltf::Model model;
	tinygltf::TinyGLTF gltfctx;
	std::string err;
	std::string warn;
	//std::string inputFilename = "C:\\Projects\\gltf-sample-models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb";
	std::string inputFilename = "C:\\Projects\\gltf-sample-models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb";
	bool ret = gltfctx.LoadBinaryFromFile(&model, &err, &warn, inputFilename.c_str());

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
	}
	dump(model);
	populate(model, globals);
}

void TinyGLTFexample::render(const RSexampleGlobal& globals) {

}

void TinyGLTFexample::dispose(const RSexampleGlobal& globals) {

}

ss::BoundingBox TinyGLTFexample::getBounds() {
	return ibbox;
}

std::string TinyGLTFexample::getExampleName() const {
	return "TinyGLTFexample";
}
