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
	static const int POINT_CIRCLE = 0;
	static const int LINE_CIRCLE = 1;
	enum PrimitiveType {
		Points,
		Lines
	};

	RSviewID iviewID;
	RScontextID ictxID;


	std::array<CircleEntity, 2> icircles;
	
	void initShaderPath(RSinitInfo& initInfo);
	void createEntity(CircleEntity& ce);

public:
	PrimitiveExample();
	void init() override;
	void render() override;
	void dispose() override;
};
