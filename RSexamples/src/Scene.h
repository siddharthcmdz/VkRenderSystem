#pragma once
#include <vector>
#include "ModelData.h"

namespace ss {
	class Scene {
	private:
		ModelData imodelData;

	public:
		Scene() = default;
		//void setModelData(const ModelData& md);
		//ModelData getModelData() const;
	};
};