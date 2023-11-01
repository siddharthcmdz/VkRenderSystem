#pragma once

#include "RSexample.h"
#include "BoundingBox.h"
#include <vector>
#include <rsids.h>
#include <TextureLoader.h>
#include "ModelData.h"

class VolumeSliceExample : public RSexample {
private:
	ss::BoundingBox ibbox;
	RSsingleEntity iquad;
	RScollectionID icollectionID;
	RStextureInfo itexInfo;
	ss::ModelData imodelData;

public:
	VolumeSliceExample();
	void init(const RSexampleOptions& eo, const RSexampleGlobal& globals) override;
	void render(const RSexampleGlobal& globals) override;
	void dispose(const RSexampleGlobal& globals) override;
	ss::BoundingBox getBounds() override;
	std::string getExampleName() const override;
};
