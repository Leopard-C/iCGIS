#version 330 core

uniform vec3 u_highlightColor;

void main()
{
	gl_FragColor = vec4(u_highlightColor, 1.0);
}
