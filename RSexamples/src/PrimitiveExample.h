#pragma once
#include "RSexample.h"
#include <vector>
#include <glm/glm.hpp>
#include <VertexData.h>

class PrimitiveExample : public RSexample {
private:
	std::vector<rsvd::VertexPC> icirclePoints;
	std::vector<rsvd::VertexPC> icircleLineStrip;
	std::vector<rsvd::VertexPC> icircleFilled;

public:
	PrimitiveExample();
	void init() override;
	void render() override;
	void dispose() override;
};
