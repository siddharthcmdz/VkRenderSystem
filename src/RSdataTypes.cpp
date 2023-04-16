#include "RSdataTypes.h"

bool RSviewID::isValid() const {
	return id != ~0;
}