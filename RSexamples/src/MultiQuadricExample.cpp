#include "MultiQuadricExample.h"


void MultiQuadricExample::init(const RSexampleOptions& eo, const RSexampleGlobal& globals) {

}

void MultiQuadricExample::render(const RSexampleGlobal& globals) {

}

void MultiQuadricExample::dispose(const RSexampleGlobal& globals) {

}

ss::BoundingBox MultiQuadricExample::getBounds() {
	return ibbox;
}

std::string MultiQuadricExample::getExampleName() const {
	return "MultiQuadricExample";
}
