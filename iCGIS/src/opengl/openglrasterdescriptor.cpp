#include "openglrasterdescriptor.h"

OpenglRasterDescriptor::~OpenglRasterDescriptor()
{
    if (vao)
        delete vao;
    if (vbo)
        delete vbo;
    if (ibo)
        delete ibo;
    for (auto& tex : texs)
        delete tex;
}
