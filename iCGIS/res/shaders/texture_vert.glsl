#version 330 core
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 u_MVP;

void main()
{
	gl_Position = u_MVP * vec4(aPosition, 0.0, 1.0);
	//texCoord = (u_MVP * vec4(aTexCoord, 0.0, 1.0)).xy;
	texCoord = aTexCoord;
}

