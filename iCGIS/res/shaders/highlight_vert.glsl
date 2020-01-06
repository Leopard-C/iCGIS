#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(position, 0.0, 1.0);
}
