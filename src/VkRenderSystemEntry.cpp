#include "VkRenderSystemEntry.h"
#include "VkRenderSystem.h"

int RSmain() {
	VkRenderSystem& vkrs = VkRenderSystem::getInstance();
	RSinitInfo info;
	sprintf_s(info.appName, "VkRSexample");
	info.enableValidation = true;
	vkrs.renderSystemInit(info);

	return 0;
}