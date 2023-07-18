#include "RSdataTypes.h"

bool RSviewID::isValid() const {
	return id != INVALID_ID;
}

bool RScontextID::isValid() const {
	return id != INVALID_ID;
}

bool RScollectionID::isValid() const {
	return id != INVALID_ID;
}