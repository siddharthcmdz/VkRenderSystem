#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include <vector>

class MultiQuadricExample : public RSexample {
private:

	ss::BoundingBox ibbox;
	std::vector<RSsingleEntity> iquadrics;

public:
	MultiQuadricExample() = default;
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	ss::BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
