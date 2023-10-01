#pragma once

#include <glm/glm.hpp>

namespace ss {
	class BoundingBox {
	private:
		glm::vec4 iminpt = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
		glm::vec4 imaxpt = glm::vec4(FLT_MIN, FLT_MIN, FLT_MIN, 1.0f);

	public:
		BoundingBox() = default;
		BoundingBox(glm::vec4 minpt, glm::vec4 maxpt);
		glm::vec4 getmin() const;
		glm::vec4 getmax() const;
		void expandBy(const glm::vec4& pt);
		bool isInside(const glm::vec4& pt) const;
	};
}

