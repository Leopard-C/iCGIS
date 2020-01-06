#include "opengl/texture.h"


Texture::Texture(void* data, int width, int height, unsigned int dataTypeIn, unsigned int internalFormatIn, unsigned int formatIn)
{
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));
	GLCall(glGenTextures(1, &textureID));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
	// wrapping parameters
	// 纹理环绕方式
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	// filtering parameters
	// 纹理过滤
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	// 生成纹理
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalFormatIn, width, height, 0, formatIn, dataTypeIn, data));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &textureID));
}

void Texture::Bind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
}

void Texture::UnBind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

