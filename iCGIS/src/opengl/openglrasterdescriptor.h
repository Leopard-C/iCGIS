/*******************************************************************
** class name:  OpenglRasterDescriptor
**
** description: A collection of VAO, VBO, IBO(s) for raster data
**
** last change: 2020-03-25
********************************************************************/
#ifndef OPENGLRASTERDESCRIPTOR_H
#define OPENGLRASTERDESCRIPTOR_H

#include "opengl/vertexarray.h"
#include "opengl/vertexbuffer.h"
#include "opengl/indexbuffer.h"
#include "opengl/texture.h"
#include <vector>


class OpenglRasterDescriptor
{
public:
    ~OpenglRasterDescriptor();

    VertexBuffer* vbo = nullptr;
    VertexArray*  vao = nullptr;
    IndexBuffer*  ibo = nullptr;
    std::vector<Texture*>  texs;
};

#endif // OPENGLRASTERDESCRIPTOR_H
