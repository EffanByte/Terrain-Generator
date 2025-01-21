#ifndef NOISE_H
#define NOISE_H

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vector>

struct TerrainData {
    std::vector<float> vertices;  // [x,y,z,  x,y,z,  x,y,z, ...]
    std::vector<unsigned int> indices;
};
float generateTransitionNoise(float worldX, float worldZ, float seed, float scale) {
    float currentFreq = 1.0f;

    // First layer of noise (base layer)
    float sampleX1 = (worldX / scale) * currentFreq;
    float sampleZ1 = (worldZ / scale) * currentFreq;
    float sampleY1 = seed * 0.5f * currentFreq;
    float baseNoise = glm::perlin(glm::vec3(sampleX1, sampleY1, sampleZ1));

    // Second layer of noise (target layer)
    float sampleX2 = (worldX / (scale * 0.5f)) * currentFreq + 100.0f;
    float sampleZ2 = (worldZ / (scale * 0.5f)) * currentFreq + 100.0f;
    float sampleY2 = (seed + 1000.0f) * 0.5f * currentFreq;
    float targetNoise = glm::perlin(glm::vec3(sampleX2, sampleY2, sampleZ2));

    // Third layer of noise (transition speed control)
    float sampleX3 = (worldX / (scale * 2.0f)) * currentFreq + 500.0f;
    float sampleZ3 = (worldZ / (scale * 2.0f)) * currentFreq + 500.0f;
    float sampleY3 = (seed + 2000.0f) * 0.5f * currentFreq;
    float transitionSpeed = glm::perlin(glm::vec3(sampleX3, sampleY3, sampleZ3));

    // Normalize and adjust the transition speed
    // Higher values = faster transition, lower values = slower transition
    transitionSpeed = (transitionSpeed + 1.0f) * 0.5f;  // Normalize to [0,1]
  //  transitionSpeed = pow(transitionSpeed, 8.0f);       // Optional: Make speed changes more dramatic

    // Calculate progress value based on some progression factor (e.g., time or position)
    float progressValue = (worldX + worldZ) / (scale * 10.0f);  // You can modify this based on your needs

    // Apply the transition speed to the progress
    float adjustedProgress = progressValue * transitionSpeed;

    // Create a smooth transition using a sigmoid-like function
    float transitionFactor = 1.0f / (1.0f + exp(-adjustedProgress));

    // Blend between base and target noise using the modified transition
    return glm::mix(baseNoise, targetNoise, transitionFactor);
}

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

                // Use seed in the Y coordinate for continuous change
                float sampleY = seed * 0.5f * currentFreq;

                float noiseValue = generateTransitionNoise(worldX, worldZ, seed, scale);

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
