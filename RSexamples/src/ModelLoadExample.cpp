#include "ModelLoadExample.h"
#include <assimp/ai_assert.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <assert.h>

ModelLoadExample::ModelLoadExample() {

}

void ModelLoadExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {

	Assimp::Importer importer;
	const std::string pFile = "C:\\Projects\\FSI\\RSexamples\\models\\monkey.glb";
	const aiScene* scene = importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace |
		aiProcess_FlipUVs|
		aiProcess_GenSmoothNormals |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);

	imdcaps.hasMesh = scene->HasMeshes();
	imdcaps.hasAnimations = scene->HasAnimations();
	imdcaps.hasCameras = scene->HasCameras();
	imdcaps.hasLights = scene->HasLights();
	imdcaps.hasMaterials = scene->HasMaterials();
	imdcaps.hasSkeleton = scene->hasSkeletons();
	imdcaps.hasTextures = scene->HasTextures();

	assert(imdcaps.hasMesh && "model file has no meshes!");

	if (nullptr == scene) {
		throw std::runtime_error(importer.GetErrorString());
	}
	
	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		if (!mesh->HasPositions()) {
			throw std::runtime_error("mesh file has no vertex positions - invalid file");
		}

		std::vector<glm::vec4> rsposlist;
		std::vector<glm::vec4> rsnormlist;
		std::vector<glm::vec2> rstexcoordlist;
		aiVector3D zero3D(0.0f, 0.0f, 0.0f);
		for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D pos = mesh->mVertices[j];
			glm::vec4 rspos(pos.x, pos.y, pos.z, 1.0f);
			rsposlist.push_back(rspos);
			
			if (mesh->HasNormals()) {
				aiVector3D* norm = mesh->mNormals != nullptr ? &(mesh->mNormals[j]) : &zero3D;
				glm::vec4 rsnorm(norm->x, norm->y, norm->z, 1.0f);
				rsnormlist.push_back(rsnorm);
			}

			if (mesh->HasTextureCoords(i)) {
				aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : &zero3D;
				glm::vec2 rstexcoord(texcoord->x, texcoord->y);
				rstexcoordlist.push_back(rstexcoord);
			}
			ibbox.expandBy(rspos);
		}

		std::vector<uint32_t> rsindexlist;
		for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
			const aiFace& face = mesh->mFaces[j];
			assert(face.mNumIndices == 3 && "tri indices is not 3");
			rsindexlist.push_back(face.mIndices[0]);
			rsindexlist.push_back(face.mIndices[1]);
			rsindexlist.push_back(face.mIndices[2]);
		}
		
		if(scene->HasMaterials()) {
			for (uint32_t j = 0; j < scene->mNumMaterials; j++) {
				const aiMaterial* material = scene->mMaterials[j];
				if (material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0) {
					aiString path;
					if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
						std::string fullpath = path.data;

					}
				}
			}
		}

	}
}

void ModelLoadExample::render(const RSexampleGlobal& globals) {

}

void ModelLoadExample::dispose(const RSexampleGlobal& globals) {

}

std::string ModelLoadExample::getExampleName() const {
	return "ModelLoadExample";
}

BoundingBox ModelLoadExample::getBounds() {
	return ibbox;
}
