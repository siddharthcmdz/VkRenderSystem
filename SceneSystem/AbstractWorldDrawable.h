//// =================================================================================
// Copyright (c) 2023, See All Surgical Inc. All rights reserved.
//
// This software is the property of See All Surgical Inc. The software may not be reproduced,
// modified, distributed, or transferred without the express written permission of See All Surgical Inc.
//
// In no event shall See All Surgical Inc. be liable for any claim, damages or other liability,
// whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software
// or the use or other dealings in the software.
// =================================================================================

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
