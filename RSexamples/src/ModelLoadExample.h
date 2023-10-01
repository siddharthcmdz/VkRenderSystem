#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include "ModelData.h"
#include "Scene.h"

using namespace ss;

struct aiMesh;
struct aiScene;
struct aiNode;

class ModelLoadExample : public RSexample {
private:
	ModelData imodelData;

	ss::MeshData getMesh(const aiMesh* mesh, uint32_t& matIdx);
	void traverseScene(const aiScene* scene, const aiNode* node, ModelData& modelData);
	void populateRSentities();
	void initRSgeomData(MeshData& meshdata);
	void initRSinstance(MeshInstance& meshinstance);
	void initRSappearance(Appearance& app);
	RSvertexAttribsInfo getRSvertexAttribData(const aiMesh* mesh);

public:
	ModelLoadExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
