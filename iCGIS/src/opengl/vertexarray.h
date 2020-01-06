/*******************************************************
** class name:  VertexArray
**
** description: 顶点数组对象
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "vertexbuffer.h"

class VertexBufferLayout;


class VertexArray {
public:
	VertexArray();
	~VertexArray();

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void Bind() const;
	void Unbind() const;
	void reserve(int count);
	void setStride(int i, int iStride) { stride[i] = iStride; }
	int getStride(int i) const { return stride[i]; }
	int getStridesCount() const { return nStrides; }

private:
	unsigned int rendererID;
	int* stride = nullptr;
	int nStrides;
};
