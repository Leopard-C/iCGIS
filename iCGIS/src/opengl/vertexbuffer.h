/******************************************************
** class name:  VertexBuffer
**
** description: VBO
**
** last change: 2020-01-02
*******************************************************/

#pragma once

class VertexBuffer {
public:
    // if data is nullptr
    // you should call addSubData(...) to add data
    // but size should be given now
    VertexBuffer(const void* data, unsigned int size, int usage = 0x88E4 /*GL_STATIC_DRAW*/);
    ~VertexBuffer();

    unsigned int getRendererID() const { return rendererID; }
    unsigned int getSize() const { return size; }

    void addSubData(const void* data, unsigned int offset, unsigned int size /*GL_STATIC_DRAW*/);

    void Bind() const;
    void Unbind() const;
private:
    unsigned int size;
    unsigned int rendererID;
};
