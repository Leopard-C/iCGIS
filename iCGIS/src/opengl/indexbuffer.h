/********************************************************************
** class name:  IndexBuffer
**
** description: IBO (or EBO in OpenGL)
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

