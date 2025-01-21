#version 330 core

out vec4 FragColor;
in vec3 position;
uniform float seaLevel;

// Improved hash function
vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

// Gradient noise (Perlin-like)
float gradientNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // Smoothing
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    // Gradients
    vec2 ga = hash2(i + vec2(0.0, 0.0));
    vec2 gb = hash2(i + vec2(1.0, 0.0));
    vec2 gc = hash2(i + vec2(0.0, 1.0));
    vec2 gd = hash2(i + vec2(1.0, 1.0));
    
    // Dot products
    float va = dot(ga, f - vec2(0.0, 0.0));
    float vb = dot(gb, f - vec2(1.0, 0.0));
    float vc = dot(gc, f - vec2(0.0, 1.0));
    float vd = dot(gd, f - vec2(1.0, 1.0));
    
    // Interpolation
    return mix(mix(va, vb, u.x),
              mix(vc, vd, u.x), u.y);
}

// Function to create smooth color transitions
vec4 smoothColor(float height, vec4 colorA, vec4 colorB, float threshold, float smoothness) {
    float t = clamp((height - threshold) / smoothness + 0.5, 0.0, 1.0);
    return mix(colorA, colorB, t);
}

// FBM (Fractal Brownian Motion) for more natural variation
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    // Add multiple layers of noise
    for(int i = 0; i < 4; i++) {
        value += amplitude * gradientNoise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

void main()
{
    // Use FBM for more natural height variation
    float noiseValue = fbm(position.xz * 3.0) * 0.3;
    float adjustedHeight = position.y + noiseValue;
    
    // Define colors with slightly adjusted alpha for better blending
    vec4 waterDeep = vec4(0.0, 0.1, 0.4, 1.0);
    vec4 waterShallow = vec4(0.0, 0.2, 0.6, 1.0);
    vec4 sandColor = vec4(0.76, 0.7, 0.5, 1.0);
    vec4 grassLight = vec4(0.3, 0.5, 0.1, 1.0);
    vec4 grassDark = vec4(0.2, 0.4, 0.1, 1.0);
    vec4 mountainColor = vec4(0.5, 0.35, 0.25, 1.0);
    vec4 snowColor = vec4(0.9, 0.9, 0.95, 1.0);

    // Add subtle variation based on additional noise layers
    float detailNoise = gradientNoise(position.xz * 10.0) * 0.1;
    
    if (adjustedHeight < seaLevel + 3) {
        FragColor = smoothColor(adjustedHeight, sandColor, grassLight, 0.3, 0.1);
}   else if (adjustedHeight < seaLevel + 6) {
    FragColor = smoothColor(adjustedHeight, grassLight, grassDark, 0.5, 0.2);
}   else if (adjustedHeight < seaLevel + 8) {
    FragColor = smoothColor(adjustedHeight, grassDark, mountainColor, 0.9, 0.2);
}   else {
    FragColor = smoothColor(adjustedHeight, mountainColor, snowColor, 1.4, 0.3);
}

    // Add subtle detail variation
    FragColor.rgb += vec3(detailNoise);
    
    // Ensure colors stay in valid range
    FragColor = clamp(FragColor, 0.0, 1.0);
}