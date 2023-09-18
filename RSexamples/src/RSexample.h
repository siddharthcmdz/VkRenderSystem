#pragma once

#include <string>
#include <rsids.h>
#include <glm/glm.hpp>
#include "BoundingBox.h"
#include <rsenums.h>

enum RSexampleName {
	enHelloVulkan,
	enPrimitiveType,
	enModelLoad,
	enMax
};

const std::string RSexampleNameStr[] = {
	"HelloVulkanExample",
	"PrimitiveExample",
	"ModelLoadExample",
	"None"
};

struct RSexampleOptions {
	RSexampleName example;
};

struct RSexampleGlobal {
	RScontextID ctxID;
	RSviewID viewID;
	uint32_t width = 0;
	uint32_t height = 0;
};

struct RSsingleEntity {
	RSgeometryDataID geomDataID;
	RSgeometryID geomID;
	RSappearanceID appID;
	RSspatialID spatialID;
	RStextureID textureID;
	RSinstanceID instanceID;
	RScollectionID collectionID;
	RSstateID stateID;
	RSprimitiveType primType;
};

class RSexample {
public:
	virtual void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) = 0;
	virtual void render(const RSexampleGlobal& globals) = 0;
	virtual void dispose(const RSexampleGlobal& globals) = 0;
	virtual ss::BoundingBox getBounds() = 0;
	virtual std::string getExampleName() const = 0;
};

