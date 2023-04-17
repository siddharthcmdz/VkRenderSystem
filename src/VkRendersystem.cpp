#include "VkRenderSystem.h"
#include <assert.h>

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
	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewCreate(const RSview& view, RSviewID& viewID)
{
	RSuint id;
	bool success = iviewIDpool.CreateID(id);
	assert(success && "failed to create a view ID");
	if (success && viewID.isValid()) {
		iviewMap[id] = view;
		viewID.id = id;
		return RSresult::SUCCESS;
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewUpdate(const RSviewID& viewID, const RSview& view)
{
	if (viewID.isValid()) {
		if (iviewMap.find(viewID.id) != iviewMap.end()) {
			iviewMap[viewID.id] = view;
			iviewMap[viewID.id].dirty = true;

			return RSresult::SUCCESS;
		}
	}

	return RSresult::FAILURE;
}

RSresult VkRenderSystem::viewDraw(const RSviewID& viewID)
{
	return RSresult::SUCCESS;
}

RSresult VkRenderSystem::viewDispose(const RSviewID& viewID)
{
	return RSresult::SUCCESS;
}
