
#include "MathUtils.h"

namespace ss {
    std::array<glm::vec3, 2> MathUtils::getOrthoNormals(glm::vec3 normal)
    {
        glm::vec3 vec1, vec2;
        if(normal == glm::vec3(0, 1, 0))
        {
            vec1 = glm::vec3(1, 0, 0);
            vec2 = glm::vec3(0, 0, 1);
        }
        else if(normal == glm::vec3(1, 0, 0))
        {
            vec1 = glm::vec3(0, 1, 0);
            vec2 = glm::vec3(0, 0, 1);
        }
        else if(normal == glm::vec3(0, 0, 1))
        {
            vec1 = glm::vec3(1, 0, 0);
            vec2 = glm::vec3(0, 1, 0);
        }
        else
        {
            //handle non orthogonal normal case
            glm::vec3 nnormal = glm::normalize(normal);
            if(nnormal[0] != 0 && nnormal[1] != 0)
            {
                vec1 = glm::vec3(-nnormal[1], nnormal[0], 0);
            }
            else if(nnormal[1] != 0 && nnormal[2] != 0)
            {
                vec1 = glm::vec3(0, -nnormal[2], nnormal[1]);
            }
            else
            {
                vec1 = glm::vec3(-nnormal[2], 0, nnormal[0]);
            }
            
            vec2 = glm::cross(nnormal, vec1);
            vec2 = glm::normalize(vec2);
        }
        
        std::array<glm::vec3, 2> orthos;
        orthos[0] = vec1;
        orthos[1] = vec2;
        
        return orthos;
    }

    std::array<glm::vec4, 4> MathUtils::getRectCorners(float width, float height, glm::vec3 normal, bool centerOrigin)
    {
        std::array<glm::vec3, 2> vecs = getOrthoNormals(normal);
        glm::vec3 vec1 = vecs[0], vec2 = vecs[1];
        glm::vec3 pt0, pt1, pt2, pt3;
        
        if(centerOrigin)
        {
            float halfWdith = width * 0.5f;
            float halfHeight = height * 0.5f;
            pt0 = (-halfWdith * vec1) + (halfHeight * vec2);
            pt1 = (halfWdith * vec1) + (halfHeight * vec2);
            pt2 = (halfWdith * vec1) + (-halfHeight * vec2);
            pt3 = (-halfWdith * vec1) + (-halfHeight * vec2);
        }
        else
        {
            pt0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            pt1 = width * vec1;
            pt2 = width * vec1 + height * vec2;
            pt3 = height * vec2;
        }
        
        std::array<glm::vec4, 4> corners{};
        corners[0] = glm::vec4(pt0, 1.0f);
        corners[1] = glm::vec4(pt1, 1.0f);
        corners[2] = glm::vec4(pt2, 1.0f);
        corners[3] = glm::vec4(pt3, 1.0f);
        
        return corners;
        
    }
}
