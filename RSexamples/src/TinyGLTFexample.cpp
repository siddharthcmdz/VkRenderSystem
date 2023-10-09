#include "TinyGLTFexample.h"

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

void TinyGLTFexample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {
	tinygltf::Model model;
	tinygltf::TinyGLTF gltfctx;
	std::string err;
	std::string warn;
	std::string inputFilename = "C:\\Projects\\gltf-sample-models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb";
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
