#version 330 core

in vec3 fragcolor;

void main()
{
	gl_FragColor = vec4(fragcolor, 1.0);
}
