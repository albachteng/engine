#pragma once
#include "Component.h"
#include <memory>

class NoiseGenerator {
private:
    static constexpr int PERMUTATION_SIZE = 256;
    int m_permutation[PERMUTATION_SIZE * 2];
    
public:
    explicit NoiseGenerator(unsigned int seed = 0);
    
    // Different noise types for different effects
    float perlin2D(float x, float y) const;
    float ridgedNoise(float x, float y, int octaves = 4) const;
    float billowNoise(float x, float y, int octaves = 4) const;
    
    // Specialized for boundary distortion
    Vec2f boundaryNoise(Vec2f point, float frequency, float amplitude) const;
    
private:
    void initializePermutation(unsigned int seed);
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y) const;
};

class BoundaryDistorter {
private:
    std::unique_ptr<NoiseGenerator> m_noise;
    
public:
    explicit BoundaryDistorter(unsigned int seed) 
        : m_noise(std::make_unique<NoiseGenerator>(seed)) {}
    
    std::vector<Vec2f> distortBoundary(const std::vector<Vec2f>& originalVertices,
                                       const Vec2f& centroid,
                                       float roughness = 0.3f,
                                       float frequency = 0.02f,
                                       int subdivisions = 2);
                                       
private:
    std::vector<Vec2f> subdivideEdges(const std::vector<Vec2f>& vertices, int levels);
    Vec2f distortVertex(const Vec2f& vertex, const Vec2f& centroid, 
                       float roughness, float frequency);
    float calculateDistanceFromCenter(const Vec2f& point, const Vec2f& center);
};