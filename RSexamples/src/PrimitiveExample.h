#pragma once
#include "RSexample.h"
#include <vector>
#include <glm/glm.hpp>
#include <VertexData.h>
#include <rsids.h>
#include <RSdataTypes.h>
#include "helper.h"
#include <array>
#include "BoundingBox.h"

using namespace ss;

struct CircleEntity : public RSsingleEntity {
	std::vector<rsvd::VertexPC> vertices;
	float radius = 0.25f;

	void dispose();
};

class PrimitiveExample : public RSexample {
private:
	BoundingBox ibbox;
	enum PrimitiveType {
		Points,
		Lines,
		Solid
	};

	std::array<CircleEntity, 3> icircles;
	std::vector<rsvd::VertexPC> getVertices(PrimitiveType pt, float radius);
	void createEntity(const RSexampleGlobal& globals, CircleEntity& ce, PrimitiveType pt);

public:
	PrimitiveExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
