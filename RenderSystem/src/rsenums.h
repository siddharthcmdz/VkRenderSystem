#pragma once

#include <string>

/**
* @brief describes what vertex attributes are being used. 
* vaPosition should always use glm::vec4 where w is reserved
* vaNormal should always use glm::vec4 where w is reserved
* vaColor should always use glm::vec4
* vaTexCoord should always use glm::vec2
**/
enum RSvertexAttribute {
	vaPosition,
	vaNormal,
	vaColor,
	vaTexCoord
};

enum RSvertexAttributeSettings {
	vasInterleaved,
	vasSeparate
};

enum RSprimitiveType {
	ptTriangle,
	ptTriangleStrip,
	ptPoint,
	ptLine,
	ptLineLoop,
	ptLineStrip
};

enum RSshaderTemplate {
	stPassthrough,
	stTextured,
	stSimpleLit
};

std::string getShaderStr(const RSshaderTemplate shaderTemplate);