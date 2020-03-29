#include "vertexbuffer.h"
#include "glcall.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size, int usage/* = GL_STATIC_DRAW */)
    : size(size)
{
    GLCall(glGenBuffers(1, &rendererID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererID));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}


VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1, &rendererID));
}

void VertexBuffer::addSubData(const void* data, unsigned int offset, unsigned int size /*GL_STATIC_DRAW*/)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererID));
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererID));
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
