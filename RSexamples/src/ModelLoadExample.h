#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include "ModelData.h"
#include "Scene.h"

using namespace ss;

struct aiMesh;
struct aiScene;
struct aiNode;
struct aiMaterial;

class ModelLoadExample : public RSexample {
private:
	ModelData imodelData;

	ss::MeshData getMesh(const aiMesh* mesh);
	void traverseScene(const aiScene* scene, const aiNode* node, ModelData& modelData);
	void populateRSentities(const RSexampleGlobal& globals);
	void initRSgeomData(MeshData& meshdata);
	void initRSinstance(MeshInstance& meshinstance);
	void initRSappearance(MeshInstance& meshInstance, Appearance& app, aiMaterial* mat, const aiScene* scene);
	RSvertexAttribsInfo getRSvertexAttribData(const aiMesh* mesh);

public:
	ModelLoadExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
