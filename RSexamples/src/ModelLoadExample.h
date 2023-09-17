#pragma once
#include "RSexample.h"
#include "BoundingBox.h"
#include "ModelCapabilities.h"

using namespace ss;

class ModelLoadExample : public RSexample {
private:
	BoundingBox ibbox;
	ss::ModelCapabilities imdcaps;

public:
	ModelLoadExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
