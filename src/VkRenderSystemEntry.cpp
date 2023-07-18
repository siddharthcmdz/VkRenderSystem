#include "VkRenderSystemEntry.h"
#include "VkRenderSystem.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>           //for uint32_t
#include <limits>            //for std::numeric_limits
#include <algorithm>         //for std::clamp
#include <fstream>           //for file reading





int RSmain() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "RenderSystem");
	info.enableValidation = true;
	info.onScreenCanvas = true;
	vkrs.renderSystemInit(info);

	RScontextID ctxID;
	RScontextInfo ctxInfo;
	ctxInfo.width = 800;
	ctxInfo.height = 600;
	sprintf_s(ctxInfo.title, "Hello Triangle");
	vkrs.contextCreate(ctxID, ctxInfo);

	RSviewID viewID;
	RSview rsview;
	rsview.clearColor = glm::vec4(0, 0, 0, 1);
	rsview.cameraType = CameraType::ORBITAL;
	vkrs.viewCreate(viewID, rsview);

	if (vkrs.isRenderSystemInit()) {
		vkrs.renderSystemDrawLoop(ctxID, viewID);
	}

	return 0;
}