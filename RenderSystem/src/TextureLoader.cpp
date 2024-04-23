////// =================================================================================
//// Copyright (c) 2023, See All Surgical Inc. All rights reserved.
////
//// This software is the property of See All Surgical Inc. The software may not be reproduced,
//// modified, distributed, or transferred without the express written permission of See All Surgical Inc.
////
//// In no event shall See All Surgical Inc. be liable for any claim, damages or other liability,
//// whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software
//// or the use or other dealings in the software.
//// =================================================================================
//
//#include "TextureLoader.h"
//#include "stb_image.h"
//#include <chrono>
//#include <random>
//#include <ctime>
//#include <algorithm>
//#include <numeric>
//#include <iostream>
//
//void RStextureInfo::dispose()
//{
//    if(this->texels != nullptr)
//    {
//        //stbi is used for loading texture 2d, so use its own methods to dispose it.
//        if(textureType == RStextureType::ttTexture2D)
//        {
//            stbi_image_free(this->texels);
//        }
//    }
//    this->texels = nullptr;
//}
//
//// Translation of Ken Perlin's JAVA implementation (http://mrl.nyu.edu/~perlin/noise/)
//template <typename T>
//class PerlinNoise
//{
//    private:
//    RStextureFormat _texformat;
//    std::vector<uint32_t> permutations;
//    T fade(T t)
//    {
//        return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
//    }
//    T lerp(T t, T a, T b)
//    {
//        return a + t * (b - a);
//    }
//    T grad(int hash, T x, T y, T z)
//    {
//        // Convert LO 4 bits of hash code into 12 gradient directions
//        int h = hash & 15;
//        T u = h < 8 ? x : y;
//        T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
//        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
//    }
//    public:
//    PerlinNoise()
//    {
//    }
//    
//    PerlinNoise(RStextureFormat texformat)
//    {
//        _texformat = texformat;
//
//        uint32_t sz = 0;
//        if(texformat == RStextureFormat::tfUnsignedBytes)
//        {
//            // Generate random lookup for permutations containing all numbers from 0..255
//            sz = 256;
//            permutations.resize(sz*2);
//            std::vector<uint8_t> plookup;
//            plookup.resize(sz);
//            std::iota(plookup.begin(), plookup.end(), 0);
//            std::default_random_engine rndEngine(std::random_device{}());
//            std::shuffle(plookup.begin(), plookup.end(), rndEngine);
//            
//            for (uint32_t i = 0; i < sz; i++)
//            {
//                permutations[i] = permutations[sz + i] = plookup[i];
//            }
//        }
//        else
//        {
//            // Generate random lookup for permutations containing all numbers from 0..65535
//            sz = 65536;
//            permutations.resize(sz*2);
//            std::vector<uint16_t> plookup;
//            plookup.resize(sz);
//            std::iota(plookup.begin(), plookup.end(), 0);
//            std::default_random_engine rndEngine(std::random_device{}());
//            std::shuffle(plookup.begin(), plookup.end(), rndEngine);
//
//            for (uint32_t i = 0; i < sz; i++)
//            {
//                permutations[i] = permutations[sz + i] = plookup[i];
//            }
//        }
//    }
//    
//    T noise(T x, T y, T z)
//    {
//        // Find unit cube that contains point
//        uint32_t sz = _texformat == RStextureFormat::tfUnsignedBytes ? 255 : 65535;
//        int32_t X = (int32_t)floor(x) & sz;
//        int32_t Y = (int32_t)floor(y) & sz;
//        int32_t Z = (int32_t)floor(z) & sz;
//        // Find relative x,y,z of point in cube
//        x -= floor(x);
//        y -= floor(y);
//        z -= floor(z);
//
//        // Compute fade curves for each of x,y,z
//        T u = fade(x);
//        T v = fade(y);
//        T w = fade(z);
//
//        // Hash coordinates of the 8 cube corners
//        uint32_t A = permutations[X] + Y;
//        uint32_t AA = permutations[A] + Z;
//        uint32_t AB = permutations[A + 1] + Z;
//        uint32_t B = permutations[X + 1] + Y;
//        uint32_t BA = permutations[B] + Z;
//        uint32_t BB = permutations[B + 1] + Z;
//
//        // And add blended results for 8 corners of the cube;
//        T res = lerp(w, lerp(v,
//            lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)), lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))),
//            lerp(v, lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)), lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))));
//        return res;
//    }
//};
//
//// Fractal noise generator based on perlin noise above
//template <typename T>
//class FractalNoise
//{
//    private:
//    PerlinNoise<float> perlinNoise;
//    uint32_t octaves;
//    T frequency;
//    T amplitude;
//    T persistence;
//    public:
//
//    FractalNoise(const PerlinNoise<T>& perlinNoise)
//    {
//        this->perlinNoise = perlinNoise;
//        octaves = 6;
//        persistence = (T)0.5;
//    }
//
//    T noise(T x, T y, T z)
//    {
//        T sum = 0;
//        T frequency = (T)1;
//        T amplitude = (T)1;
//        T max = (T)0;
//        for (uint32_t i = 0; i < octaves; i++)
//        {
//            sum += perlinNoise.noise(x * frequency, y * frequency, z * frequency) * amplitude;
//            max += amplitude;
//            amplitude *= persistence;
//            frequency *= (T)2;
//        }
//
//        sum = sum / max;
//        return (sum + (T)1.0) / (T)2.0;
//    }
//};
//
//RStextureInfo TextureLoader::create3Dvolume(uint32_t width, uint32_t height, uint32_t depth, RStextureFormat textureFormat)
//{
//    RStextureInfo texInfo;
//    texInfo.width = width;
//    texInfo.height = height;
//    texInfo.depth = depth;
//    texInfo.numChannels = 1;
//    texInfo.textureType = RStextureType::ttTexture3D;
//    texInfo.texelFormat = textureFormat;
//    
//    const uint32_t texMemSize = texInfo.width * texInfo.height * texInfo.depth;
//    
//    
//    uint8_t* data8 = nullptr;
//    uint16_t* data16 = nullptr;
//    std::string textureFormatStr = "Invalid";
//    if(texInfo.texelFormat == RStextureFormat::tfUnsignedBytes)
//    {
//        data8 = new uint8_t[texMemSize];
//        memset(data8, 0, texMemSize);
//        textureFormatStr = " 8 bit";
//    }
//    else
//    {
//        data16 = new uint16_t[texMemSize];
//        memset(data16, 0, texMemSize);
//        textureFormatStr = " 16 bit";
//    }
//
//    // Generate perlin based noise
//    std::cout << "Generating " << texInfo.width << " x " << texInfo.height << " x " << texInfo.depth << textureFormatStr<<" noise volume..." << std::endl;
//
//    auto tStart = std::chrono::high_resolution_clock::now();
//    
//    PerlinNoise<float> perlinNoise(texInfo.texelFormat);
//    FractalNoise<float> fractalNoise(perlinNoise);
//
//    const float noiseScale = static_cast<float>(rand() % 10) + 4.0f;
//    
//    for (uint32_t z = 0; z < texInfo.depth; z++)
//    {
//        for (uint32_t y = 0; y < texInfo.height; y++)
//        {
//            for (uint32_t x = 0; x < texInfo.width; x++)
//            {
//                float nx = (float)x / (float)texInfo.width;
//                float ny = (float)y / (float)texInfo.height;
//                float nz = (float)z / (float)texInfo.depth;
//#define FRACTAL
//#ifdef FRACTAL
//                float n = fractalNoise.noise(nx * noiseScale, ny * noiseScale, nz * noiseScale);
//#else
//                float n = 20.0 * perlinNoise.noise(nx, ny, nz);
//#endif
//                n = n - floor(n);
//                
//                if(texInfo.texelFormat == RStextureFormat::tfUnsignedBytes)
//                {
//                    data8[x + y * texInfo.width + z * texInfo.width * texInfo.height] = static_cast<uint8_t>(floor(n * 255));
//                }
//                else
//                {
//                    data16[x + y * texInfo.width + z * texInfo.width * texInfo.height] = static_cast<uint16_t>(floor(n * 65535));
//                }
//            }
//        }
//    }
//    
//    if(texInfo.texelFormat == RStextureFormat::tfUnsignedBytes)
//    {
//        texInfo.texels = data8;
//    }
//    else
//    {
//        texInfo.texels = data16;
//    }
//    auto tEnd = std::chrono::high_resolution_clock::now();
//    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
//
//    std::cout << "Done in " << tDiff << "ms" << std::endl;
//
//    return texInfo;
//}
//
//RStextureInfo TextureLoader::readTexture(const char* filepath) 
//{
//    RStextureInfo ti;
//    int widthi = static_cast<int>(ti.width);
//    int heighti = static_cast<int>(ti.height);
//    int numchannelsi = static_cast<int>(ti.numChannels);
//    ti.texels = stbi_load(filepath, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);
//
//    return ti;
//}
//
//RStextureInfo TextureLoader::readFromMemory(unsigned char* texdata, uint32_t width, uint32_t height) 
//{
//    RStextureInfo ti;
//    int widthi = static_cast<int>(ti.width);
//    int heighti = static_cast<int>(ti.height);
//    int numchannelsi = static_cast<int>(ti.numChannels);
//
//    if (height == 0)
//    {
//        ti.texels = stbi_load_from_memory(texdata, width, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);
//    }
//    else
//    {
//        ti.texels = stbi_load_from_memory(texdata, width * height, &widthi, &heighti, &numchannelsi, STBI_rgb_alpha);
//    }
//
//    return ti;
//}
