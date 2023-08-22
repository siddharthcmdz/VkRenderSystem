#pragma once
#include "RSexample.h"
#include <vector>
#include <glm/glm.hpp>
#include <VertexData.h>
#include <rsids.h>
#include <RSdataTypes.h>
#include "helper.h"
#include <array>

struct CircleEntity : public helper::RSsingleEntity {
	std::vector<rsvd::VertexPC> vertices;
	float radius = 0.25f;

	void dispose();
};

class PrimitiveExample : public RSexample {
private:
	enum PrimitiveType {
		Points,
		Lines,
		Solid
	};

	RSviewID iviewID;
	RScontextID ictxID;


	std::array<CircleEntity, 3> icircles;
	std::vector<rsvd::VertexPC> getVertices(PrimitiveType pt, float radius);

	void initShaderPath(RSinitInfo& initInfo);
	void createEntity(CircleEntity& ce, PrimitiveType pt);

public:
	PrimitiveExample();
	void init() override;
	void render() override;
	void dispose() override;
};
