#version 330 core

uniform vec3 u_borderColor;

void main()
{
	gl_FragColor = vec4(u_borderColor, 1.0);
}
