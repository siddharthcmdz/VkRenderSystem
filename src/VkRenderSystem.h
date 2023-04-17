#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"

class VkRenderSystem {

private:
	std::unordered_map<uint32_t ,RSview> iviewMap;
	MakeID iviewIDpool = MakeID(MAX_IDS);
	
public:
	static VkRenderSystem& getInstance() {
		static VkRenderSystem INSTANCE;

		return INSTANCE;
	}

	RSresult renderSystemInit();
	bool isRenderSystemInit();
	RSresult renderSystemDispose();

	RSresult viewCreate(const RSview& view, RSviewID& viewID);
	RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RSresult viewDraw(const RSviewID& viewID);
	RSresult viewDispose(const RSviewID& viewID);
};