#include "VkRenderSystem.h"

RSresult VkRenderSystem::renderSystemInit()
{
	return RSresult::FAILURE;
}

bool VkRenderSystem::isRenderSystemInit()
{
	return false;
}

RSresult VkRenderSystem::renderSystemDispose()
{
	return RSresult();
}

RSresult VkRenderSystem::viewCreate(const RSview& view, RSviewID& viewID)
{
	
	return RSresult();
}

RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view)
{
	return RSresult();
}

RSresult VkRenderSystem::viewDraw(const RSviewID& viewID)
{
	return RSresult();
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	return RSresult();
}
