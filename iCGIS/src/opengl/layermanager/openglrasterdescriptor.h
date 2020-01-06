/*******************************************************
** class name:  OpenglRasterDescriptor
**
** description: 栅格数据描述符
**
** last change: 2020-01-05
*******************************************************/
#pragma once

#include "opengl/vertexarray.h"
#include "opengl/vertexbuffer.h"
#include "opengl/indexbuffer.h"
#include "opengl/texture.h"
#include "memoryleakdetect.h"


class OpenglRasterDescriptor {
public:
	OpenglRasterDescriptor() {}
	~OpenglRasterDescriptor() {
		if (vbo) delete vbo;
		if (vao) delete vao;
		if (ibo) delete ibo;
		for (auto& tex : texs) delete tex;
	}

	void setVBO(VertexBuffer* vboIn) { vbo = vboIn; }
	void setVAO(VertexArray* vaoIn)  { vao = vaoIn; }
	void setIBO(IndexBuffer* iboIn)  { ibo = iboIn; }
	void addTex(Texture* texIn) { texs.push_back(texIn); }

	VertexBuffer* vbo = nullptr;
	VertexArray*  vao = nullptr;
	IndexBuffer*  ibo = nullptr;
	std::vector<Texture*>  texs;
};

