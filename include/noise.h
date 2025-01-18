#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vector>
#include <cmath>

struct TerrainData {
    std::vector<float> vertices;  // [x,y,z,  x,y,z,  x,y,z, ...]
    std::vector<unsigned int> indices;
};

// Function to calculate a smooth falloff factor
float calculateFalloffFactor(int x, int z, int width, int height) {
    float edgeDistanceX = std::min(x, width - 1 - x) / (float)(width / 2);
    float edgeDistanceZ = std::min(z, height - 1 - z) / (float)(height / 2);
    float edgeDistance = std::min(edgeDistanceX, edgeDistanceZ);

    // Apply a smooth curve to the falloff factor (e.g., quadratic or sigmoid)
    return 1 / (exp(-edgeDistance) + 1); // Sigmoid falloff
}

TerrainData generateTerrain(int width, int height, float scale, float seed, int octaves, float persistence, float frequency, float lacunarity, float heightScale, float xOffset = 0.0f, float zOffset = 0.0f) {
    TerrainData terrain;
    terrain.vertices.reserve(width * height * 3); // Reserve memory for vertices

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            // Adjust world positions with global offsets
            float worldX = ((float)x + xOffset) - (width / 2.0f);
            float worldZ = ((float)z + zOffset) - (height / 2.0f);

            // Accumulate multiple octaves of noise
            float heightValue = 0.0f;
            float amplitude = 1.0f;
            float maxValue = 0.0f;

            for (int o = 0; o < octaves; o++) {
                float currentFreq = frequency * pow(lacunarity, o); // Adjust frequency using lacunarity
                float sampleX = (worldX / scale) * currentFreq;
                float sampleZ = (worldZ / scale) * currentFreq;

                // Incorporate seed for consistent randomness
                float sampleY = seed * 0.5f * currentFreq;

                float noiseValue = glm::perlin(glm::vec3(sampleX, sampleY, sampleZ));
                heightValue += noiseValue * amplitude;

                maxValue += amplitude;
                amplitude *= persistence; // Reduce amplitude with persistence
            }

            // Normalize and scale the height
            heightValue = (heightValue / maxValue) * heightScale;

            // Apply falloff factor for smooth edges
            float falloffFactor = calculateFalloffFactor(x, z, width, height);
            heightValue *= falloffFactor;

            // Store the vertex data
            terrain.vertices.push_back(worldX);     // x-coordinate
            terrain.vertices.push_back(heightValue); // y-coordinate (height)
            terrain.vertices.push_back(worldZ);     // z-coordinate
        }
    }

    // Generate indices for triangle-based mesh
    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            terrain.indices.push_back(topLeft);
            terrain.indices.push_back(bottomLeft);
            terrain.indices.push_back(topRight);

            terrain.indices.push_back(topRight);
            terrain.indices.push_back(bottomLeft);
            terrain.indices.push_back(bottomRight);
        }
    }

    return terrain;
}


#endif
