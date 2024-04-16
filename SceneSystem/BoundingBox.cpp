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

#include "BoundingBox.h"
#include <cfloat>
#include <algorithm>

namespace ss 
{
    BoundingBox::BoundingBox(glm::vec4 minpt, glm::vec4 maxpt) 
    {
        _minpt = minpt;
        _maxpt = maxpt;
        _isValid = true;
    }

    glm::vec4 BoundingBox::getmin() const 
    {
        return _minpt;
    }

    glm::vec4 BoundingBox::getmax() const 
    {
        return _maxpt;
    }

    void BoundingBox::expandBy(const glm::vec4& pt) 
    {
        _minpt.x = pt.x < _minpt.x ? pt.x : _minpt.x;
        _minpt.y = pt.y < _minpt.y ? pt.y : _minpt.y;
        _minpt.z = pt.z < _minpt.z ? pt.z : _minpt.z;

        _maxpt.x = pt.x > _maxpt.x ? pt.x : _maxpt.x;
        _maxpt.y = pt.y > _maxpt.y ? pt.y : _maxpt.y;
        _maxpt.z = pt.z > _maxpt.z ? pt.z : _maxpt.z;
    }

    void BoundingBox::expandBy(float pad) 
    {
        _minpt.x -= pad;
        _minpt.y -= pad;
        _minpt.z -= pad;
        
        _maxpt.x += pad;
        _maxpt.y += pad;
        _maxpt.z += pad;
    }

    void BoundingBox::expandBy(const BoundingBox& bbox)
    {
        expandBy(bbox.getmin());
        expandBy(bbox.getmax());
    }

    float BoundingBox::getDiagonal() const 
    {
        return glm::length(_maxpt - _minpt);
    }

    bool BoundingBox::isInside(const glm::vec4& pt) const 
    {
        return  _minpt.x <= pt.x && pt.x <= _maxpt.x &&
                _minpt.y <= pt.y && pt.y <= _maxpt.y &&
                _minpt.z <= pt.z && pt.z <= _maxpt.z;
    }

    glm::vec3 BoundingBox::getCenter() const
    {
        return (_maxpt + _minpt) * 0.5f;
    }

    BoundingBox BoundingBox::xform(const glm::mat4& xform) const
    {
        glm::vec4 xformedMin = xform * _minpt;
        glm::vec4 xformedMax = xform * _maxpt;
        
        return BoundingBox(xformedMin, xformedMax);
    }

    bool BoundingBox::isValid() const
    {
        return _isValid;
    }
}
