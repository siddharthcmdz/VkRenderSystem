#pragma once
#include <vector>
#include "MeshInstance.h"

namespace ss {
	class Scene {
	private:
		std::vector<MeshInstance> imeshInstanceList;
		
	public:
		Scene() = default;
		void addMeshInstance(const MeshInstance& meshInst);
		~Scene();
	};
};