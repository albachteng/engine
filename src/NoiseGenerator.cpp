#include "../include/NoiseGenerator.h"
#include "../include/Logger.hpp"
#include <cmath>
#include <random>
#include <algorithm>

// NoiseGenerator implementation
NoiseGenerator::NoiseGenerator(unsigned int seed) {
    initializePermutation(seed);
}

void NoiseGenerator::initializePermutation(unsigned int seed) {
    // Initialize with standard permutation table
    int basePermutation[PERMUTATION_SIZE] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
        140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
        247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
        57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
        74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
        60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
        65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
        200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
        52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
        207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
        119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
        218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
        81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
        184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
        222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };
    
    // Shuffle based on seed
    std::mt19937 rng(seed);
    std::vector<int> temp(basePermutation, basePermutation + PERMUTATION_SIZE);
    std::shuffle(temp.begin(), temp.end(), rng);
    
    // Create doubled permutation table
    for (int i = 0; i < PERMUTATION_SIZE; ++i) {
        m_permutation[i] = temp[i];
        m_permutation[i + PERMUTATION_SIZE] = temp[i];
    }
    
    LOG_DEBUG_STREAM("NoiseGenerator: Initialized with seed " << seed);
}

float NoiseGenerator::perlin2D(float x, float y) const {
    // Find grid cell containing point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    
    // Relative coordinates within cell
    x -= std::floor(x);
    y -= std::floor(y);
    
    // Compute fade curves for x and y
    float u = fade(x);
    float v = fade(y);
    
    // Hash coordinates of 4 cell corners
    int A = m_permutation[X] + Y;
    int AA = m_permutation[A];
    int AB = m_permutation[A + 1];
    int B = m_permutation[X + 1] + Y;
    int BA = m_permutation[B];
    int BB = m_permutation[B + 1];
    
    // Blend results from 4 corners
    return lerp(v, 
        lerp(u, grad(m_permutation[AA], x, y),
                grad(m_permutation[BA], x - 1, y)),
        lerp(u, grad(m_permutation[AB], x, y - 1),
                grad(m_permutation[BB], x - 1, y - 1)));
}

float NoiseGenerator::ridgedNoise(float x, float y, int octaves) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        float value = 1.0f - std::abs(perlin2D(x * frequency, y * frequency));
        value = value * value; // Square for sharper ridges
        total += value * amplitude;
        
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

float NoiseGenerator::billowNoise(float x, float y, int octaves) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        float value = std::abs(perlin2D(x * frequency, y * frequency));
        total += value * amplitude;
        
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

Vec2f NoiseGenerator::boundaryNoise(Vec2f point, float frequency, float amplitude) const {
    float noiseX = perlin2D(point.x * frequency, point.y * frequency);
    float noiseY = perlin2D((point.x + 100.0f) * frequency, (point.y + 100.0f) * frequency);
    
    return Vec2f(noiseX * amplitude, noiseY * amplitude);
}

float NoiseGenerator::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float NoiseGenerator::lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

float NoiseGenerator::grad(int hash, float x, float y) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// BoundaryDistorter implementation
std::vector<Vec2f> BoundaryDistorter::distortBoundary(const std::vector<Vec2f>& originalVertices,
                                                      const Vec2f& centroid,
                                                      float roughness,
                                                      float frequency,
                                                      int subdivisions) {
    if (originalVertices.empty()) return {};
    
    LOG_DEBUG_STREAM("BoundaryDistorter: Distorting boundary with " << originalVertices.size() 
                    << " vertices, roughness=" << roughness << ", frequency=" << frequency);
    
    // First, subdivide edges to create more vertices for smoother distortion
    std::vector<Vec2f> subdividedVertices = subdivideEdges(originalVertices, subdivisions);
    
    // Apply noise distortion to each vertex
    std::vector<Vec2f> distortedVertices;
    distortedVertices.reserve(subdividedVertices.size());
    
    for (const auto& vertex : subdividedVertices) {
        Vec2f distortedVertex = distortVertex(vertex, centroid, roughness, frequency);
        distortedVertices.push_back(distortedVertex);
    }
    
    LOG_DEBUG_STREAM("BoundaryDistorter: Created " << distortedVertices.size() << " distorted vertices");
    return distortedVertices;
}

std::vector<Vec2f> BoundaryDistorter::subdivideEdges(const std::vector<Vec2f>& vertices, int levels) {
    if (levels <= 0 || vertices.size() < 3) return vertices;
    
    std::vector<Vec2f> result = vertices;
    
    for (int level = 0; level < levels; ++level) {
        std::vector<Vec2f> newVertices;
        
        for (size_t i = 0; i < result.size(); ++i) {
            size_t nextI = (i + 1) % result.size();
            
            // Add current vertex
            newVertices.push_back(result[i]);
            
            // Add midpoint
            Vec2f midpoint;
            midpoint.x = (result[i].x + result[nextI].x) * 0.5f;
            midpoint.y = (result[i].y + result[nextI].y) * 0.5f;
            newVertices.push_back(midpoint);
        }
        
        result = newVertices;
    }
    
    return result;
}

Vec2f BoundaryDistorter::distortVertex(const Vec2f& vertex, const Vec2f& centroid, 
                                       float roughness, float frequency) {
    // Calculate direction from centroid to vertex
    Vec2f direction = Vec2f(vertex.x - centroid.x, vertex.y - centroid.y);
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (distance < 0.001f) return vertex; // Avoid division by zero
    
    // Normalize direction
    direction.x /= distance;
    direction.y /= distance;
    
    // Generate noise value
    Vec2f noiseOffset = m_noise->boundaryNoise(vertex, frequency, roughness * distance * 0.5f);
    
    // Apply distortion primarily in radial direction with some tangential component
    Vec2f tangent(-direction.y, direction.x); // Perpendicular to radial
    
    Vec2f distortion;
    distortion.x = direction.x * noiseOffset.x + tangent.x * noiseOffset.y * 0.3f;
    distortion.y = direction.y * noiseOffset.x + tangent.y * noiseOffset.y * 0.3f;
    
    return Vec2f(vertex.x + distortion.x, vertex.y + distortion.y);
}

float BoundaryDistorter::calculateDistanceFromCenter(const Vec2f& point, const Vec2f& center) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return std::sqrt(dx * dx + dy * dy);
}