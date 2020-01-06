/***********************************************************************
** class name:  OpenglFeatureDescriptor
**
** description: 要素描述符，在OpenGLWidget中，向GPU发送要素数据时，
**				会返回一个要素描述符，记录要素的FID和与之相关的VAO、
**				VBO、IBO等
**
** last change: 2020-01-02
************************************************************************/
#pragma once

#include "opengl/vertexarray.h"
#include "opengl/vertexbuffer.h"
#include "opengl/indexbuffer.h"
#include "geo/geometry/geogeometry.h"
#include "memoryleakdetect.h"

#include <vector>


class OpenglFeatureDescriptor 
{
public:
	OpenglFeatureDescriptor() {}
	OpenglFeatureDescriptor(int nFID, GeometryType typeIn)
		: FID(nFID), type(typeIn) {}
	~OpenglFeatureDescriptor() {
		if (vbo)
			delete vbo;
		if (vao)
			delete vao;
		for (auto& ibo : ibos)
			delete ibo;
	}

	void setFID(int nFID) { FID = nFID; }
	void setType(GeometryType typeIn) { type = typeIn; }
	void setSelected(bool selected) { isSelected = selected; }

	void setVBO(VertexBuffer* vboIn) { vbo = vboIn; }
	void setVAO(VertexArray* vaoIn) { vao = vaoIn; }
	void addIBO(IndexBuffer* iboIn) { ibos.push_back(iboIn); }

	VertexBuffer* vbo = nullptr;
	VertexArray* vao = nullptr;
	std::vector<IndexBuffer*> ibos;

	int FID;
	GeometryType type;
	bool isSelected = false;
};
