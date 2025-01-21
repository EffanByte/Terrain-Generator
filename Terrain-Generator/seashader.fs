#version 330 core
out vec4 FragColor;
uniform float seaLevel;
void main()
{
    // Set the fragment color to blue
    FragColor = vec4(0.0, 0.3, 0.8, 1.0); // RGBA: Red, Green, Blue, Alpha
}
