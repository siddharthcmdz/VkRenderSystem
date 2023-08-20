
#include <conio.h>

#include "HelloVulkanExample.h"
#include "PrimitiveExample.h"

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

const RSexampleOptions& processArgs(int argc, char** argv) {
	RSexampleOptions exopts;
	for (int i = 1; i < argc; i++) {
		std::string mode = argv[i];
		if (mode == "-example") {
			std::string exname = argv[(i + 1)];
			for (int j = 0; j < RSexampleName::enMax; j++) {
				if (exname == RSexampleNameStr[j]) {
					exopts.example = static_cast<RSexampleName>(j);
					break;
				}
			}
		}
	}
	return exopts;
}

int main(int argc, char** argv) {
	const RSexampleOptions& exopts = processArgs(argc, argv);
	RSexample* example = nullptr;
	switch (exopts.example) {
	case RSexampleName::enHelloVulkan:
		example = new HelloVulkanExample();
		break;

	case RSexampleName::enPrimitiveType:
		example = new PrimitiveExample();
		break;

	case RSexampleName::enMax:
	default:
		example = new HelloVulkanExample();
	}
	
	example->init();
	example->render();
	example->dispose();
	getchar();
	return 0;
}