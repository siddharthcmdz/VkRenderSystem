#include "Bounds.h"
#include <cfloat>

BoundingBox::BoundingBox(glm::vec4 minpt, glm::vec4 maxpt) {
	iminpt = minpt;
	imaxpt = maxpt;
}

glm::vec4 BoundingBox::getmin() const {
	return iminpt;
}

glm::vec4 BoundingBox::getmax() const {
	return imaxpt;
}

void BoundingBox::expandBy(const glm::vec4& pt) {
	
}

void BoundingBox::shrinkBy(const glm::vec4& pt) {

}

bool BoundingBox::isInside(const glm::vec4& pt) const {
	return false;
}
