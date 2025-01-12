#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


struct TerrainData {
    std::vector<float> vertices;  // [x,y,z,  x,y,z,  x,y,z, ...]
    std::vector<unsigned int> indices;
};


TerrainData generateTerrain(int width, int height, float scale, float seed, int octaves, int persistence, float frequency) {
    TerrainData terrain;
    terrain.vertices.reserve(width * height * 3);

    // Parameters for more interesting terrain
    const float heightScale = 10.0f;  // Amplify the height variation
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            float worldX = (float)x - (width / 2.0f);
            float worldZ = (float)z - (height / 2.0f);

            // Accumulate multiple octaves of noise
            float heightValue = 0.0f;
            float amplitude = 1.0f;
            float maxValue = 0.0f;

            for (int o = 0; o < octaves; o++) {
                // Sample perlin noise with seed component
                float sampleX = (worldX / scale) * frequency;
                float sampleZ = (worldZ / scale) * frequency;

                // Use seed in the Y coordinate for continuous change
                float sampleY = seed * 0.8f * frequency;

                float noiseValue = glm::perlin(glm::vec3(sampleX, sampleY, sampleZ));
                heightValue += noiseValue * amplitude;

                maxValue += amplitude;
                amplitude *= persistence;
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
