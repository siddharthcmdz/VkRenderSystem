#pragma once
#include <vector>
#include "MeshInstance.h"

namespace ss {
	class Scene {
	private:
		std::vector<MeshInstance> meshInstance;
		
	public:
		Scene() = default;
		void addMeshInstance(const MeshInstance& meshInst);
		~Scene();
	};
};