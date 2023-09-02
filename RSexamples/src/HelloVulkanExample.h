#pragma once
#include "RSexample.h"
#include <vector>
#include <VertexData.h>
#include "helper.h"

class HelloVulkanExample : public RSexample {
	const std::vector<rsvd::VertexPCT> ivertices = {
		{{-0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	const std::vector<uint32_t> iindices = {
		0, 1, 2, 2, 3, 0
	};

	helper::RSsingleEntity ientity;

public:
	HelloVulkanExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	std::string getExampleName() const override;
};