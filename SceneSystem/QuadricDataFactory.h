
#pragma once

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "BoundingBox.h"

struct QuadricData 
{
    std::vector<glm::vec4> positions{};
    std::vector<glm::vec4> normals{};
    std::vector<glm::vec4> colors{};
    std::vector<glm::vec2> texcoords{};
    std::vector<uint32_t> indices;
    ss::BoundingBox bbox;
};

class QuadricDataFactory 
{
private:
    const static float RS_PI;
    const static float DEFAULT_NUM_STACKS;
    const static float DEFAULT_NUM_SLICES;
    const static float DEFAULT_RADIUS;
    const static float DEFAULT_PHI_MAX;
    const static float DEFAULT_SIZE;
    const static float DEFAULT_HEIGHT;
    const static float DEFAULT_HALF_RADIUS;

public:
    
    /**
     * @brief Constructs a parameteric and configurable sphere  quadric
     * @param radius the specified radius
     * @param numslices  the number of vertical longitudes
     * @param numstacks the number of horizontal latitudes or stacks
     * @return the quadric data that contains all vertex attributes and indices.
     */
    static QuadricData createSphere(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_STACKS, uint32_t numstacks = DEFAULT_NUM_SLICES);
    
    /**
     * @brief Constructs a quad of given size.
     * @param size the specified size of the quad.
     * @param normal the specified normal to the quad
     * @return the quadric data that contains all vertex attributes and indices.
     */
    static QuadricData createQuad(float size = DEFAULT_SIZE, glm::vec3 normal = glm::vec3(0, 1, 0));
    
    /**
     * @brief Constructs a parametric and configurable cone.
     * @param radius the specified radius
     * @param numslices  the number of vertical longitudes
     * @param numstacks the number of horizontal latitudes or stacks
     * @param height the specified height
     * @param phimax the specified maximum phi angle
     * @return the quadric data that contains all vertex attributes and indices.
     */
    static QuadricData createCone(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float height = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
    
    /**
     * @brief Constructs a parametric and configurable cylinder.
     * @param radius the specified radius
     * @param numslices  the number of vertical longitudes
     * @param numstacks the number of horizontal latitudes or stacks
     * @param zmin the specified zmin height
     * @param zmax the speciffied zmax height
     * @return the quadric data that contains all vertex attributes and indices.
     */
    static QuadricData createCylinder(float radius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float zmin = 0, float zmax = DEFAULT_HEIGHT, float phimax = DEFAULT_PHI_MAX);
    
    /**
     * @brief Construct a parameteric and configurable disk.
     * @param innerRadius the specified inner radius of the disk.
     * @param outerRadius the specified outer radius of the diskl.
     * @param numslices  the number of vertical longitudes
     * @param numstacks the number of horizontal latitudes or stacks
     * @param z the specified z distance from origin
     * @param phimax the specified maximum phi angle
     */
    static QuadricData createDisk(float innerRadius = DEFAULT_HALF_RADIUS, float outerRadius = DEFAULT_RADIUS, uint32_t numslices = DEFAULT_NUM_SLICES, uint32_t numstacks = DEFAULT_NUM_STACKS, float z = 0, float phimax = DEFAULT_PHI_MAX);
};
