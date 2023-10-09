#pragma once

#include "RSexample.h"
#include "BoundingBox.h"

class TinyGLTFexample : public RSexample {
private:
	ss::BoundingBox ibbox;

public:
	TinyGLTFexample() = default;
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals);
	void render(const RSexampleGlobal& globals);
	void dispose(const RSexampleGlobal& globals);
	ss::BoundingBox getBounds();
	std::string getExampleName() const;
};