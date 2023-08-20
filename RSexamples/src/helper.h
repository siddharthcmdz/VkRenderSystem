#pragma once
#include <rsids.h>
#include <Windows.h>
#include <string>

namespace helper {
	struct RSsingleEntity {
		RSgeometryDataID geomDataID;
		RSgeometryID geomID;
		RSappearanceID appID;
		RSspatialID spatialID;
		RStextureID textureID;
		RSinstanceID instanceID;
		RScollectionID collectionID;
	};


	std::string getCurrentDir();

	std::string toString(std::wstring wstr);
}
