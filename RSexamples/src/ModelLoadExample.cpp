#include "ModelLoadExample.h"
#include <assimp/ai_assert.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <stdexcept>
#include <assert.h>

ModelLoadExample::ModelLoadExample() {
}

glm::mat4 convertMat(const aiMatrix4x4* from)
{
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
	to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
	to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
	to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;

	return to;
}


void ModelLoadExample::traverseScene(const aiScene* scene, const aiNode* node, ModelData& modelData) {
	aiMatrix4x4 m = node->mTransformation;
	aiTransposeMatrix4(&m);
	MeshInstance meshInstance;
	meshInstance.modelmat = convertMat(&m);

	uint32_t n = 0;
	for (; n < node->mNumMeshes; n++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];
		meshInstance.meshData = getMesh(mesh, meshInstance.materialIdx);
		modelData.meshInstances.push_back(meshInstance);
	}

	for (n = 0; n < node->mNumChildren; ++n) {
		traverseScene(scene, node->mChildren[n], modelData);
	}
}

ss::MeshData ModelLoadExample::getMesh(const aiMesh* mesh, uint32_t& matIdx) {
	ss::MeshData meshdata;
	
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

		if (mesh->HasTextureCoords(0)) {
			aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &zero3D;
			glm::vec2 rstexcoord(texcoord->x, texcoord->y);
			rstexcoordlist.push_back(rstexcoord);
		}
	}
	meshdata.positions = rsposlist;
	meshdata.normals = rsnormlist;
	meshdata.texcoords = rstexcoordlist;
	matIdx = mesh->mMaterialIndex;

	std::vector<uint32_t> rsindexlist;
	for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
		const aiFace& face = mesh->mFaces[j];
		assert(face.mNumIndices == 3 && "tri indices is not 3");
		rsindexlist.push_back(face.mIndices[0]);
		rsindexlist.push_back(face.mIndices[1]);
		rsindexlist.push_back(face.mIndices[2]);
	}

	return meshdata;
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

	imodelData.imdcaps.hasMesh = scene->HasMeshes();
	imodelData.imdcaps.hasAnimations = scene->HasAnimations();
	imodelData.imdcaps.hasCameras = scene->HasCameras();
	imodelData.imdcaps.hasLights = scene->HasLights();
	imodelData.imdcaps.hasMaterials = scene->HasMaterials();
	imodelData.imdcaps.hasSkeleton = scene->hasSkeletons();
	imodelData.imdcaps.hasTextures = scene->HasTextures();

	assert(imodelData.imdcaps.hasMesh && "model file has no meshes!");

	if (nullptr == scene) {
		throw std::runtime_error(importer.GetErrorString());
	}

	std::vector<Appearance> materials;
	if (scene->HasMaterials()) {
		Appearance app;
		for (uint32_t j = 0; j < scene->mNumMaterials; j++) {
			const aiMaterial* material = scene->mMaterials[j];
			aiColor4D diffuse;
			aiColor4D specular;
			aiColor4D ambient;
			aiColor4D emission;
			ai_real shininess, strength;

			if (material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0) {
				aiString path;
				if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
					app.diffuseTexturePath = path.data;
				}
			}

			if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
				app.diffuse = glm::vec4(diffuse.r, diffuse.g, diffuse.b, 1.0f);
			}
			if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular)) {
				app.specular = glm::vec4(specular.r, specular.g, specular.b, 1.0f);
			}
			if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient)) {
				app.ambient = glm::vec4(ambient.r, ambient.g, ambient.b, 1.0f);
			}
			if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission)) {
				app.emissive = glm::vec4(emission.r, emission.g, emission.b, 1.0f);
			}
			uint32_t max = 1;
			int ret1 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &shininess, &max);
			if (ret1 == AI_SUCCESS) {
				max = 1;
				int ret2 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
				if (ret2 == AI_SUCCESS) {
					app.shininess = shininess * strength;
				}
				else {
					app.shininess = shininess;
				}
			}

			materials.push_back(app);
		}
	}
	imodelData.materials = materials;

	traverseScene(scene, scene->mRootNode, imodelData);
}

void ModelLoadExample::render(const RSexampleGlobal& globals) {

}

void ModelLoadExample::dispose(const RSexampleGlobal& globals) {

}

std::string ModelLoadExample::getExampleName() const {
	return "ModelLoadExample";
}

BoundingBox ModelLoadExample::getBounds() {
	return imodelData.bbox;
}
