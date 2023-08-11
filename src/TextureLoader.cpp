#include "TextureLoader.h"
#include "stb_image.h"

void RStextureInfo::dispose() {
	stbi_image_free(this->texels);
}

RStextureInfo TextureLoader::readTexture(const char* filepath) {
	RStextureInfo ti;
	ti.texels = stbi_load(filepath, &ti.texWidth, &ti.texHeight, &ti.texChannels, STBI_rgb_alpha);

	return ti;
}