#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include <vector>
#include <rsids.h>
#include "ModelData.h"

class QuadricExample : public RSexample {
private:
	
	ss::BoundingBox ibbox;
	ss::ModelData imodelData;
	RScollectionID icollectionID;

public:
	QuadricExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	ss::BoundingBox getBounds() override;
	std::string getExampleName() const override;

};
