#pragma once
#include "RSexample.h"
#include <vector>
#include <glm/glm.hpp>
#include <VertexData.h>
#include <rsids.h>

class PrimitiveExample : public RSexample {
private:
	RSviewID iviewID;
	RScollectionID icollectionID;

	struct CircleData {
		std::vector<rsvd::VertexPC> vertices;
		uint32_t radius;
		RSgeometryDataID pcGeomDataID;
		RSgeometryID pcGeomID;
		RSappearanceID pcAppID;
		RSspatialID pcSpatialID;
		RSinstanceID pcInstanceID;
	};

	CircleData icd;

public:
	PrimitiveExample();
	void init() override;
	void render() override;
	void dispose() override;
};
