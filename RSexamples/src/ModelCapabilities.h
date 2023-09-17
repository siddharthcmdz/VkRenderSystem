#pragma once

namespace ss {
	struct ModelCapabilities {
		bool hasMesh = false;
		bool hasMaterials = false;
		bool hasTextures = false;
		bool hasAnimations = false;
		bool hasSkeleton = false;
		bool hasLights = false;
		bool hasCameras = false;
	};
}
