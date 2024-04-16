#include "RSdataTypes.h"
#include <glm/glm.hpp>
#include <stdexcept>

uint32_t RSvertexAttribsInfo::sizeOfInterleavedAttrib() const {
    if (attributes == nullptr) {
        return 0;
    }

    uint32_t sum = 0;
    for (uint32_t i = 0; i < numVertexAttribs; i++) {
        RSvertexAttribute attrib = attributes[i];
        sum += sizeOfAttrib(attrib);
    }

    if (sum == 0) {
        throw std::runtime_error("size of one attribute cannot be zero!");
    }

    return sum;
}

uint32_t RSvertexAttribsInfo::sizeOfAttrib(RSvertexAttribute attrib) const {
    uint32_t sz = 0;
    switch (attrib) {
        case RSvertexAttribute::vaPosition:
            sz = sizeof(glm::vec4);
            break;

        case RSvertexAttribute::vaNormal:
            sz = sizeof(glm::vec4);
            break;

        case RSvertexAttribute::vaColor:
            sz = sizeof(glm::vec4);
            break;

        case RSvertexAttribute::vaTexCoord:
            sz = sizeof(glm::vec2);
            break;
    }
    
    if (sz == 0) {
        throw std::runtime_error("invalid size of a vertex attribute");
    }
    
    return sz;
}

std::string RSvertexAttribsInfo::getName(RSvertexAttribute attrib) const {
    std::string attribName;
    switch (attrib) {
        case RSvertexAttribute::vaPosition:
            attribName = "POSITION";
            break;

        case RSvertexAttribute::vaNormal:
            attribName = "NORMAL";
            break;

        case RSvertexAttribute::vaColor:
            attribName = "COLOR";
            break;

        case RSvertexAttribute::vaTexCoord:
            attribName = "TEXCOORD";
            break;

        default:
            attribName = "INVALID";
    }

    return attribName;
}

uint32_t RSvertexAttribsInfo::getBindingPoint(RSvertexAttribute attrib) const {
    return static_cast<uint32_t>(attrib);
}

