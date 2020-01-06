/*******************************************************
** class name:  Texture
**
** description: 纹理、贴图
**
** last change: 2020-01-05
*******************************************************/
#pragma once

#include "glcall.h"


class Texture {
public:
	Texture(void* data, int width, int height, unsigned int dataTypeIn,
		unsigned int internalFormatIn, unsigned int formatIn);
	~Texture();

	void Bind() const;
	void UnBind() const;

	unsigned int getID() const { return textureID; }

private:
	unsigned int textureID;
};
