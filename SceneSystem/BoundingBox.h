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

#include <glm/glm.hpp>

namespace ss {
    /**
     * @brief a utility class for describing a bounding box.
     */
    class BoundingBox final {
    private:
        glm::vec4 _minpt = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
        glm::vec4 _maxpt = glm::vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
        bool _isValid = false;
        
    public:
        
        /**
         * @brief a compiler provided constructor
         */
        BoundingBox() = default;
        
        /**
         * @brief Constructs a bounding box given the min and max extent points.
         */
        BoundingBox(glm::vec4 minpt, glm::vec4 maxpt);
        
        /**
         * @brief Gets the min point of the bounding box.
         * @return the min point of the box.
         */
        glm::vec4 getmin() const;
        
        /**
         * @brief Gets the max point of the bounding biox.
         * @return the max point of the box.
         */
        glm::vec4 getmax() const;
        
        /**
         * @brief Expands the bounding box given a point. If the point is already inside the bounding box, then bounding box does not change. Otherwise the bounds expands when the point is outside the bounding box,
         * @param pt the specified point to grown the bounding box to.
         */
        void expandBy(const glm::vec4& pt);
        
        /**
         * @brief Expands the bounding box given a floating scalar as a pad across all dimensions.
         * @param pad the specified padding across all dimensions
         */
        void expandBy(float pad);
            
        /**
         * @brief Expands the bounding box given another bounding box.
         * @param bbox the specified bounding box.
         */
        void expandBy(const BoundingBox& bbox);
        
        /**
         * @brief calculates the gets the diagonal distance between the bounding box's min and max point.
         * @return the distance between the min and max point.
         */
        float getDiagonal() const;
        
        /**
         * @brief Validates if a point is inside the bounding box.
         * @return true if the point is inside the box, false otherwise.
         */
        bool isInside(const glm::vec4& pt) const;
        
        /**
         * @brief Gets the center of the bounding box.
         * @return the center of the bounding box.
         */
        glm::vec3 getCenter() const;
        
        /**
         * @brief Transforms the bounding box by a matrix
         * @return the new transformed bounding box.
         */
        BoundingBox xform(const glm::mat4& xform) const;
        
        /**
         * @brief Gets if the bounding box is valid
         * @return true if valid, false otherwise
         */
        bool isValid() const;
    };
}


