/******************************************************
** class name:  VertexBuffer
**
** description: 顶点缓冲对象 
**
** last change: 2020-01-02
*******************************************************/

#pragma once

class VertexBuffer {
public:
	/* 如果data为nullptr，则构造函数中不绑定数据，而是需要后续多次调用addSubData() */
	/* size 为各部分子数据的size之和，需要预先确定 */
	VertexBuffer(const void* data, unsigned int size, int usage = 0x88E4 /*GL_STATIC_DRAW*/);
	~VertexBuffer();

	unsigned int getRendererID() const { return rendererID; }
	unsigned int getSize() const { return size; }

	// 添加子数据
	void addSubData(const void* data, unsigned int offset, unsigned int size /*GL_STATIC_DRAW*/);

	void Bind() const;
	void Unbind() const;
private:
	unsigned int size;
	unsigned int rendererID;
};

