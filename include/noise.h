#ifndef NOISE_H
#define NOISE_H
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vector>
#include <algorithm>

// Struct to hold biome-specific parameters
struct BiomeParameters {
    float heightScale;
    float frequency;
    float persistence;
    float lacunarity;
};

// Define different biome types
enum BiomeType {
    PLAINS,
    HILLS,
    MOUNTAINS,
    DESERT
};

void smoothHeights(std::vector<float>& vertices, int width, int height, int smoothingPasses = 1) {
    std::vector<float> smoothedHeights(vertices.size() / 3, 0.0f); // Only store Y-values

    for (int pass = 0; pass < smoothingPasses; pass++) {
        for (int z = 0; z < height; z++) {
            for (int x = 0; x < width; x++) {
                int index = z * width + x;
                float sum = 0.0f;
                int count = 0;

                // Check neighbors within bounds
                for (int dz = -1; dz <= 1; dz++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        int nz = z + dz;
                        if (nx >= 0 && nx < width && nz >= 0 && nz < height) {
                            int neighborIndex = nz * width + nx;
                            sum += vertices[neighborIndex * 3 + 1]; // Y-component
                            count++;
                        }
                    }
                }

                // Average the neighboring heights
                smoothedHeights[index] = sum / count;
            }
        }

        // Update the original vertices with smoothed heights
        for (int z = 0; z < height; z++) {
            for (int x = 0; x < width; x++) {
                int index = z * width + x;
                vertices[index * 3 + 1] = smoothedHeights[index];
            }
        }
    }
}


float calculateFalloff(int x, int z, int width, int height, float seaLevel) {
    int borderThreshold = 50; // Distance from the border to start falloff
    int left = x;
    int right = width - 1 - x;
    int top = z;
    int bottom = height - 1 - z;

    int distanceToEdge = std::min({ left, right, top, bottom });

    if (distanceToEdge >= borderThreshold) {
        return 1.0f; // No falloff
    }

    float normalizedDistance = (float)distanceToEdge / borderThreshold;
    return glm::mix(seaLevel, 1.0f, normalizedDistance); // Interpolate between sea level and full height
}



// Struct to hold terrain data
struct TerrainData {
    std::vector<float> vertices;  // [x,y,z, x,y,z, x,y,z, ...]
    std::vector<unsigned int> indices;
    std::vector<BiomeType> biomeMap; // Store biome type for each vertex
};

// Fuzzy logic membership function
float calculateMembership(float value, float start, float peak, float end) {
    if (value < start || value > end) return 0.0f;
    if (value < peak) {
        return (value - start) / (peak - start);
    }
    return (end - value) / (end - peak);
}

// Get biome parameters based on fuzzy membership
BiomeParameters getBiomeParameters(float noiseValue) {
    BiomeParameters params;

    // Default parameters
    params.heightScale = 50.0f;
    params.frequency = 1.0f;
    params.persistence = 0.5f;
    params.lacunarity = 2.0f;

    // Plains membership (0.0 - 0.5)
    float plainsMembership = calculateMembership(noiseValue, 0.0f, 0.25f, 0.5f);

    // Hills membership (0.5 - 0.75)
    float hillsMembership = calculateMembership(noiseValue, 0.5f, 0.625f, 0.75f);

    // Mountains membership (0.75 - 1.0)
    float mountainsMembership = calculateMembership(noiseValue, 0.75f, 0.875f, 1.0f);


    // Blend parameters based on membership values
    BiomeParameters finalParams;
    float totalMembership = plainsMembership + hillsMembership + mountainsMembership;

    if (totalMembership > 0.0f) {
        finalParams.heightScale =
            (plainsMembership * 30.0f +
                hillsMembership * 60.0f +
                mountainsMembership * 100.0f) / totalMembership;

        finalParams.frequency =
            (plainsMembership * 0.8f +
                hillsMembership * 1.2f +
                mountainsMembership * 1.5f) / totalMembership;

        finalParams.persistence =
            (plainsMembership * 0.4f +
                hillsMembership * 0.5f +
                mountainsMembership * 0.6f) / totalMembership;

        finalParams.lacunarity =
            (plainsMembership * 1.5f +
                hillsMembership * 2.0f +
                mountainsMembership * 2.5f) / totalMembership;
    }

    else {
        return params; // Return default if no membership
    }

    return finalParams;
}

TerrainData generateTerrain(int width, int height, float scale, float seed, int octaves) {
    TerrainData terrain;
    terrain.vertices.reserve(width * height * 3);
    terrain.biomeMap.reserve(width * height);

    // First pass: Generate biome noise
    std::vector<float> biomeNoise;
    biomeNoise.reserve(width * height);

    for (int z = 0; z < height; z++) {
        float worldZ = (float)z - (height / 2.0f);
        for (int x = 0; x < width; x++) {
            float worldX = (float)x - (width / 2.0f);

            // Generate biome noise using different frequency and seed
            float biomeX = (worldX / (scale * 4.0f)); // Larger scale for smoother biome transitions
            float biomeZ = (worldZ / (scale * 4.0f));
            float biomeValue = glm::perlin(glm::vec3(biomeX, seed * 0.1f, biomeZ));
            biomeNoise.push_back((biomeValue + 1.0f) * 0.5f); // Normalize to 0-1
        }
    }

    // Second pass: Generate terrain using biome parameters
    for (int z = 0; z < height; z++) {
        float worldZ = (float)z - (height / 2.0f);
        for (int x = 0; x < width; x++) {
            float worldX = (float)x - (width / 2.0f);

            // Get biome parameters based on noise
            int index = z * width + x;
            BiomeParameters biomeParams = getBiomeParameters(biomeNoise[index]);

            // Generate height using biome parameters
            float heightValue = 0.0f;
            float amplitude = 1.0f;
            float maxValue = 0.0f;

            for (int o = 0; o < octaves; o++) {
                float currentFreq = biomeParams.frequency * pow(biomeParams.lacunarity, o);
                float sampleX = (worldX / scale) * currentFreq;
                float sampleZ = (worldZ / scale) * currentFreq;
                float sampleY = seed * 0.5f * currentFreq;

                float noiseValue = glm::perlin(glm::vec3(sampleX, sampleY, sampleZ));
                heightValue += noiseValue * amplitude;
                maxValue += amplitude;
                amplitude *= biomeParams.persistence;
            }

            // Normalize and scale height based on biome
            float falloffFactor = calculateFalloff(x, z, width, height, 0.0f); // Replace 0.0f with the desired sea level height
            heightValue = (heightValue / maxValue) * biomeParams.heightScale * falloffFactor;

            // Store vertex data
            terrain.vertices.push_back(worldX);
            terrain.vertices.push_back(heightValue);
            terrain.vertices.push_back(worldZ);

            // Store biome type excluding Mountains
            if (biomeNoise[index] < 0.3f) terrain.biomeMap.push_back(PLAINS);
            else if (biomeNoise[index] < 0.55f) terrain.biomeMap.push_back(HILLS);
            else terrain.biomeMap.push_back(DESERT);  // Replace Mountain range
        }
    }

    // Generate indices (unchanged from original)
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
    smoothHeights(terrain.vertices, width, height, 3); // Apply 3 smoothing passes

    return terrain;
}
#endif