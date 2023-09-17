#include "Scene.h"

namespace ss {
	void Scene::addMeshInstance(const MeshInstance& meshInst) {
		imeshInstanceList.push_back(meshInst);
	}
	
	Scene::~Scene() {
		imeshInstanceList.clear();
	}
}