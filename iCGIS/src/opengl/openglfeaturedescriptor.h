/*******************************************************************
** class name:  OpenglFeatureDescriptor
**
** description: A collection of VAO, VBO, IBO(s)
**
** last change: 2020-03-25
********************************************************************/
#ifndef OPENGLFEATUREDESCRIPTOR_H
#define OPENGLFEATUREDESCRIPTOR_H

#include "opengl/vertexarray.h"
#include "opengl/vertexbuffer.h"
#include "opengl/indexbuffer.h"
#include "geo/geometry/geogeometry.h"

class OpenglFeatureDescriptor
{
public:
    OpenglFeatureDescriptor(int stride) : stride(stride) {}
    ~OpenglFeatureDescriptor();

    void offset(double xOffset, double yOffset);

    //void rotate();    // Just resend data to GPU

public:
    int stride;
    VertexBuffer* vbo = nullptr;
    VertexArray* vao = nullptr;
    std::vector<IndexBuffer*> ibos;
};

#endif // OPENGLFEATUREDESCRIPTOR_H
