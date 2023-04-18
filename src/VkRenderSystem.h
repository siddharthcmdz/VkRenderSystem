#pragma once
#include "RStypes.h"
#include "RSdataTypes.h"
#include "VkRSdataTypes.h"
#include <vector>
#include <unordered_map>
#include "MakeID.h"

class VkRenderSystem {

private:
	VkRSinstance iinstance;
	std::unordered_map<uint32_t ,RSview> iviewMap;
	MakeID iviewIDpool = MakeID(MAX_IDS);
	
	bool checkValidationLayerSupport() const;
	std::vector<const char*> getRequiredExtensions(const RSinitInfo& info) const;
	void populateInstanceData(VkRSinstance& inst, const RSinitInfo& info);
	void initInstance(const RSinitInfo& info);

public:
	static VkRenderSystem& getInstance() {
		static VkRenderSystem INSTANCE;

		return INSTANCE;
	}

	RSresult renderSystemInit(const RSinitInfo& info);
	bool isRenderSystemInit();
	RSresult renderSystemDispose();

	RSresult viewCreate(const RSview& view, RSviewID& viewID);
	RSresult viewUpdate(const RSviewID& viewID, const RSview& view);
	RSresult viewDraw(const RSviewID& viewID);
	RSresult viewDispose(const RSviewID& viewID);
};