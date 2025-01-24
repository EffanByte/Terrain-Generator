#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float u_time;

void main()
{
      float y = sin(aPos.x*0.5 + u_time);
    gl_Position = projection * view * model * vec4(aPos.x, y + aPos.y, aPos.z, 1.0);
}