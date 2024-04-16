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
#include "BoundingBox.h"
#include "rsids.h"
#include <vector>
#include <unordered_map>
#include <string>
#include "ssenums.h"
#include "SSdataTypes.h"

namespace ss 
{

    std::string getDrawableName(const DrawableType dt);

    /**
     * @brief A world drawable is a construct that encapsulates the entire world that is drawn in the scene. There can be only one world drawable. The idea is that different permutations of entities that can be drawn in a world is captured and can be rendered anytime. This can be used for graphics tests, benchmark purposes in addition to the real world data that the application loads.
     */
    class WorldDrawable 
    {
    public:
        
        /**
         * @brief Initialzes the geometry, view collections and instances . All of the rendersystem constructs are shared across all views.
         * @return true if initialization succeeded, false otherwise.
         */
        virtual bool init() = 0;
        
        /**
         * @brief Disposes the world drawable and all the rendersystem constructs housed within.
         * @return true if disposal was successfull, false otherwise.
         */
        virtual bool dispose() = 0;

        /**
         * @brief Gets the bounding box of the world.
         * @return the bounding box of the world.
         */
        virtual BoundingBox getBounds() = 0;
        
        /**
         * @brief Gets the collections created by the drawable.
         * @return returns the collections created by the drawable.
         */
        virtual std::vector<RScollectionID> getCollections() const = 0;
        
        /**
         * @brief Gets the name of this drawable that is short but descriptive that uniquely identifies itself amongst multiple world drawables.
         * @return the string that decribes the name of this world drawable.
         */
        virtual std::string getName() const = 0;
        
        /**
         * @brief Gets the drawable type enum value
         * @return the drawable type enum
         */
        virtual DrawableType getType() const = 0;
    };
}
