#include "BoundingBox.h"
#include <cfloat>
#include <algorithm>

namespace ss {
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
		iminpt.x = pt.x < iminpt.x ? pt.x : iminpt.x;
		iminpt.y = pt.y < iminpt.y ? pt.y : iminpt.y;
		iminpt.z = pt.z < iminpt.z ? pt.z : iminpt.z;

		imaxpt.x = pt.x > imaxpt.x ? pt.x : imaxpt.x;
		imaxpt.y = pt.y > imaxpt.y ? pt.y : imaxpt.y;
		imaxpt.z = pt.z > imaxpt.z ? pt.z : imaxpt.z;

	}

	bool BoundingBox::isInside(const glm::vec4& pt) const {
		return  iminpt.x <= pt.x && pt.x <= imaxpt.x &&
				iminpt.y <= pt.y && pt.y <= imaxpt.y &&
				iminpt.z <= pt.z && pt.z <= imaxpt.z;
	}
}
