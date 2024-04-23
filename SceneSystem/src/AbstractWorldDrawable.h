
#pragma once

#include "WorldDrawable.h"

namespace ss
{
    /**
     * @brief An abstract world drawable that contains method that is common and not needed to be implemented by all drawables.
     */
    class AbstractWorldDrawable : public WorldDrawable
    {
    public:
        void validate(SceneData& sopts) /*override*/;
    };

}
