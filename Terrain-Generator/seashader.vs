#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float u_time;
uniform float sea_frequency;
uniform float sea_amplitude;
uniform float wave_speed;

void main()
{
    // Use gl_VertexID to compute unique time for each vertex
    float vertexTime = u_time + gl_VertexID * 0.001f;

    // Create multiple waves with individual time and random variations
    float wave1 = sea_amplitude * sin(aPos.z * (sea_frequency + 0.1f) + vertexTime);
    float wave2 = (sea_amplitude * 0.8f) * sin(aPos.z * (sea_frequency + 0.15f) + vertexTime * 0.8f);
    float wave3 = (sea_amplitude * 0.9f) * sin(aPos.z * (sea_frequency + 0.2f) + vertexTime * 1.5f);
    float wave4 = (sea_amplitude * 0.6f) * sin(aPos.z * (sea_frequency + 0.05f) + vertexTime * 2.0f);
    
    // Sum the waves to create the final displacement
    float wave = wave1 + wave2 + wave3 + wave4;
    
    // Apply the displacement to the y-coordinate
    float displacedY = aPos.y + wave;
    
    // Set the final position of the vertex
    gl_Position = projection * view * model * vec4(aPos.x, displacedY, aPos.z, 1.0);
}
