#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include <vector>
#include <rsids.h>

class QuadricExample : public RSexample {
private:
	
	ss::BoundingBox ibbox;
	std::vector<RSsingleEntity> iquadrics;
	RScollectionID icollectionID;

public:
	QuadricExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	ss::BoundingBox getBounds() override;
	std::string getExampleName() const override;

};
