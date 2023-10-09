#include "ModelLoadExample.h"
#include <assimp/ai_assert.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <stdexcept>
#include <assert.h>
#include <VkRenderSystem.h>
#include <TextureLoader.h>

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
		meshInstance.meshData = getMesh(mesh);
		meshInstance.materialIdx = mesh->mMaterialIndex;
		if (modelData.materials.find(meshInstance.materialIdx) == modelData.materials.end()) {
			Appearance app;
			initRSappearance(meshInstance, app, scene->mMaterials[meshInstance.materialIdx], scene);
			modelData.materials[meshInstance.materialIdx] = app;
		}
		modelData.meshInstances.push_back(meshInstance);
	}

	for (n = 0; n < node->mNumChildren; ++n) {
		traverseScene(scene, node->mChildren[n], modelData);
	}
}

RSvertexAttribsInfo ModelLoadExample::getRSvertexAttribData(const aiMesh* mesh) {
	RSvertexAttribsInfo attribsInfo;
	std::vector<RSvertexAttribute> attribs;

	if (mesh->HasPositions()) {
		attribs.push_back(RSvertexAttribute::vaPosition);
	}
	if (mesh->HasNormals()) {
		attribs.push_back(RSvertexAttribute::vaNormal);
	}
	if (mesh->HasVertexColors(0)) {
		attribs.push_back(RSvertexAttribute::vaColor);
	}
	if (mesh->HasTextureCoords(0)) {
		attribs.push_back(RSvertexAttribute::vaTexCoord);
	}
	attribsInfo.settings = RSvertexAttributeSettings::vasSeparate;
	
	attribsInfo.numVertexAttribs = static_cast<uint32_t>(attribs.size());
	attribsInfo.attributes = new RSvertexAttribute[attribsInfo.numVertexAttribs]();
	std::memcpy(attribsInfo.attributes, attribs.data(), attribsInfo.numVertexAttribs * sizeof(RSvertexAttribute));

	return attribsInfo;
}

ss::MeshData ModelLoadExample::getMesh(const aiMesh* mesh) {
	ss::MeshData meshdata;
	meshdata.attribsInfo = getRSvertexAttribData(mesh);
	if (!mesh->HasPositions()) {
		throw std::runtime_error("mesh file has no vertex positions - invalid file");
	}
	
	std::vector<glm::vec4> rsposlist;
	std::vector<glm::vec4> rsnormlist;
	std::vector<glm::vec4> rscolorlist;
	std::vector<glm::vec2> rstexcoordlist;
	aiVector3D zero3D(0.0f, 0.0f, 0.0f);
	aiColor4D defaultColor(1.0f, 0.0f, 0.0f, 1.0f);
	for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
		aiVector3D pos = mesh->mVertices[j];
		glm::vec4 rspos(pos.x, pos.y, pos.z, 1.0f);
		rsposlist.push_back(rspos);

		if (mesh->HasNormals()) {
			aiVector3D* norm = mesh->mNormals != nullptr ? &(mesh->mNormals[j]) : &zero3D;
			glm::vec4 rsnorm(norm->x, norm->y, norm->z, 0.0f);
			rsnorm = glm::normalize(rsnorm);
			rsnormlist.push_back(rsnorm);
		}
		if(mesh->HasVertexColors(0)) {
			aiColor4D* color = mesh->mColors != nullptr ? &(mesh->mColors[0][j]) : &defaultColor;
			glm::vec4 rscolor(color->r, color->g, color->b, 1.0f);
			rscolorlist.push_back(rscolor);
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
	meshdata.colors = rscolorlist;
	std::vector<uint32_t> rsindexlist;
	for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
		const aiFace& face = mesh->mFaces[j];
		assert(face.mNumIndices == 3 && "tri indices is not 3");
		rsindexlist.push_back(face.mIndices[0]);
		rsindexlist.push_back(face.mIndices[1]);
		rsindexlist.push_back(face.mIndices[2]);
	}
	meshdata.iindices = rsindexlist;

	initRSgeomData(meshdata);
	
	return meshdata;
}

void ModelLoadExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {

	Assimp::Importer importer;
	//const std::string pFile = "C:\\Projects\\FSI\\RSexamples\\models\\monkey.glb";
	//const std::string pFile = "C:\\Projects\\gltf-sample-models\\2.0\\BoxVertexColors\\glTF-Binary\\BoxVertexColors.glb";
	const std::string pFile = "C:\\Projects\\gltf-sample-models\\2.0\\DamagedHelmet\\glTF-Binary\\DamagedHelmet.glb";
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

	traverseScene(scene, scene->mRootNode, imodelData);

	//populate rs stuff
	populateRSentities(globals);
	auto& vkrs = VkRenderSystem::getInstance();
	assert(imodelData.collectionID.isValid() && "model data collection ID should be valid at this point");
	vkrs.viewAddCollection(globals.viewID, imodelData.collectionID);
}

void ModelLoadExample::initRSgeomData(MeshData& meshdata) {
	auto& vkrs = VkRenderSystem::getInstance();
	
	assert(!meshdata.positions.empty() && "mesh must have atleast position");
	uint32_t numvertices = static_cast<uint32_t>(meshdata.positions.size());
	uint32_t numindices = static_cast<uint32_t>(meshdata.iindices.size());
	vkrs.geometryDataCreate(meshdata.geometryDataID, numvertices, numindices, meshdata.attribsInfo);

	for (uint32_t i = 0; i < meshdata.attribsInfo.numVertexAttribs; i++) {
		RSvertexAttribute attrib = meshdata.attribsInfo.attributes[i];
		vkrs.geometryDataUpdateVertices(meshdata.geometryDataID, 0, numvertices * meshdata.attribsInfo.sizeOfAttrib(attrib), attrib, meshdata.getAttribData(attrib));
	}

	if (!meshdata.iindices.empty()) {
		vkrs.geometryDataUpdateIndices(meshdata.geometryDataID, 0, numindices * sizeof(uint32_t), meshdata.iindices.data());
	}

	vkrs.geometryDataFinalize(meshdata.geometryDataID);

	RSgeometryInfo geomInfo;
	geomInfo.primType = RSprimitiveType::ptTriangle;
	vkrs.geometryCreate(meshdata.geometryID, geomInfo);
}

void ModelLoadExample::initRSappearance(MeshInstance& meshInstance, Appearance& app, aiMaterial* material, const aiScene* scene) {
	auto& vkrs = VkRenderSystem::getInstance();
	aiColor4D diffuse;
	aiColor4D specular;
	aiColor4D ambient;
	aiColor4D emission;
	ai_real shininess, strength;
	if (material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0) {
		uint32_t textureCount = material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);
		for (uint32_t i = 0; i < textureCount; i++) {
			aiString path;
			if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
				const aiTexture* tex = scene->GetEmbeddedTexture(path.data);
				RSresult texres;
				if (tex != nullptr) {
					app.isDiffuseTextureEmbedded = true;
					texres = vkrs.textureCreateFromMemory(meshInstance.diffuseTextureID, reinterpret_cast<unsigned char*>(tex->pcData), tex->mWidth, tex->mHeight);
				}
				else {
					app.diffuseTexturePath = path.data;
					texres = vkrs.textureCreate(meshInstance.diffuseTextureID, path.data);
				}
			}
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

	RSappearanceInfo appInfo;
	appInfo.diffuseTexture = meshInstance.diffuseTextureID;
	appInfo.shaderTemplate = RSshaderTemplate::stSimpleLit;
	vkrs.appearanceCreate(meshInstance.appearanceID, appInfo);
}

void ModelLoadExample::initRSinstance(MeshInstance& mi) {
	auto& vkrs = VkRenderSystem::getInstance();
	
	RSinstanceInfo instinfo;
	instinfo.gdataID = mi.meshData.geometryDataID;
	instinfo.geomID = mi.meshData.geometryID;
	
	RSspatial spatial;
	spatial.model = mi.modelmat;
	spatial.modelInv = glm::inverse(spatial.model);
	vkrs.spatialCreate(mi.spatialID, spatial);
	
	Appearance& app = imodelData.materials[mi.materialIdx];
	
	instinfo.spatialID = mi.spatialID;
	instinfo.appID = mi.appearanceID;

	vkrs.collectionInstanceCreate(imodelData.collectionID, mi.instanceID, instinfo);
}

void ModelLoadExample::populateRSentities(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();

	uint32_t numinsts = static_cast<uint32_t>(imodelData.meshInstances.size());
	RScollectionInfo collinfo;
	collinfo.maxInstances = 1000;
	vkrs.collectionCreate(imodelData.collectionID, collinfo);

	for (uint32_t i = 0; i < numinsts; i++) {
		MeshData& md = imodelData.meshInstances[i].meshData;
		//initRSgeomData(md);
		MeshInstance& mi = imodelData.meshInstances[i];
		mi.associatedCollectionID = imodelData.collectionID;
		initRSinstance(mi);
	}

	vkrs.collectionFinalize(imodelData.collectionID, globals.ctxID, globals.viewID);
}

void ModelLoadExample::render(const RSexampleGlobal& globals) {
	auto& vkrs = VkRenderSystem::getInstance();

	vkrs.contextDrawCollections(globals.ctxID, globals.viewID);
}

void ModelLoadExample::dispose(const RSexampleGlobal& globals) {
	uint32_t numMeshes = static_cast<uint32_t>(imodelData.meshInstances.size());
	for (uint32_t i = 0; i < numMeshes; i++) {
		MeshInstance& mi = imodelData.meshInstances[i];
		mi.dispose();
	}

	uint32_t numAppearances = static_cast<uint32_t>(imodelData.materials.size());
	for (uint32_t i = 0; i < numAppearances; i++) {
		Appearance& app = imodelData.materials[i];
		app.dispose();
	}

	auto& vkrs = VkRenderSystem::getInstance();
	vkrs.renderSystemDispose();
}

std::string ModelLoadExample::getExampleName() const {
	return "ModelLoadExample";
}

BoundingBox ModelLoadExample::getBounds() {
	return imodelData.bbox;
}
