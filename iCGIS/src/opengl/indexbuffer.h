/********************************************************************
** class name:  IndexBuffer
**
** description: 索引缓冲对象，（在OpenGL中称为EBO，但是IBO更明确一些）
**
** last change: 2020-01-02
********************************************************************/
#pragma once

class IndexBuffer {
public:
	IndexBuffer(const unsigned int* data, unsigned int count, int mode);
	~IndexBuffer();
	void Bind() const;
	void Unbind() const;

	inline unsigned int getCount() const { return count; }
	inline int getMode() const { return mode; }
private:
	unsigned int rendererID;
	unsigned int count;
	int mode;
};

