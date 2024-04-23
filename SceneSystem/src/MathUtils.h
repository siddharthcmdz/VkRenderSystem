#pragma once
#include <array>
#include <glm/glm.hpp>

namespace ss {

    /**
     * @brief A common repository of math function used across different classes in scene system.
     */
    class MathUtils final
    {
    private:
        MathUtils();
        
    public:
        
        /**
         * @brief calculates two vectors that is normal to the given normal vector that defines the plane of hte quad
         * @param normal the specified normal vector
         * @return two vectors that are orthogonal to each other and to the normal.
         */
        static std::array<glm::vec3, 2> getOrthoNormals(glm::vec3 normal);
        
        /**
         * @brief calculates 4 points of a quad whose dimensions are equal to input size and given normal to the plane.
         * @param size the specified size of the quad in width and height
         * @param normal the specified normal of the quad
         * @return the four points of the quad.
         */
        static std::array<glm::vec4, 4> getRectCorners(float size, glm::vec3 normal);
        
        /**
         * @brief calculates 4 points of a quad whose dimensions are equal to input size and given normal to the plane.
         * @param width the  specified width of the slice
         * @param height the specified height of the slice
         * @param normal the specified normal of the quad
         * @param centerOrigin if true the rectangle will be centered at 0, 0, 0. else the rect's min pt is centered at 0, 0, 0. This is useful for creating volume slice by setting it to false.
         * @return the four points of the quad.
         */
        static std::array<glm::vec4, 4> getRectCorners(float width, float height, glm::vec3 normal, bool centerOrigin);
    };

}
