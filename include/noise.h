#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vector>

struct TerrainData {
    std::vector<float> vertices;  // [x,y,z,  x,y,z,  x,y,z, ...]
    std::vector<unsigned int> indices;
};

TerrainData generateTerrain(int width, int height, float scale, float seed, int octaves, float persistence, float frequency, float lacunarity, float heightScale) {
    TerrainData terrain;
    terrain.vertices.reserve(width * height * 3);

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {

                
            float worldX = (float)x - (width / 2.0f);
            float worldZ = (float)z - (height / 2.0f);

            // Accumulate multiple octaves of noise
            float heightValue = 0.0f;
            float amplitude = 1.0f;
            float maxValue = 0.0f;

            for (int o = 0; o < octaves; o++) {
                float currentFreq = frequency * pow(lacunarity, o); // Use lacunarity to adjust frequency
                // Sample Perlin noise with seed component
                float sampleX = (worldX / scale) * currentFreq;
                float sampleZ = (worldZ / scale) * currentFreq;

                // Use seed in the Y coordinate for continuous change
                float sampleY = seed * 0.5f * currentFreq;

                float noiseValue = glm::perlin(glm::vec3(sampleX, sampleY, sampleZ));
                heightValue += noiseValue * amplitude;

                maxValue += amplitude;
                amplitude *= persistence; // Decrease amplitude with persistence
            }

            // Normalize and scale the height
            heightValue = (heightValue / maxValue) * heightScale;

            terrain.vertices.push_back(worldX);
            terrain.vertices.push_back(heightValue);
            terrain.vertices.push_back(worldZ);
        }
    }

    // Indices remain the same
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
