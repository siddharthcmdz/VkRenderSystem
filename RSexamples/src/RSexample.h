#pragma once

#include <string>
#include <rsids.h>
#include <glm/glm.hpp>


enum RSexampleName {
	enHelloVulkan,
	enPrimitiveType,
	enMax
};

const std::string RSexampleNameStr[] = {
	"HelloVulkan",
	"PrimitiveType",
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

class RSexample {
public:
	virtual void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) = 0;
	virtual void render(const RSexampleGlobal& globals) = 0;
	virtual void dispose(const RSexampleGlobal& globals) = 0;

	virtual std::string getExampleName() const = 0;
};

