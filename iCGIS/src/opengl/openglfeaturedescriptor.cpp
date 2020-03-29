#include "openglfeaturedescriptor.h"
#include "opengl/glcall.h"

OpenglFeatureDescriptor::~OpenglFeatureDescriptor()
{
    if (vao)
        delete vao;
    if (vbo)
        delete vbo;
    for (auto& ibo : ibos)
        delete ibo;
}

void OpenglFeatureDescriptor::offset(double xOffset, double yOffset) {
    unsigned int count = this->vbo->getSize() / sizeof(float);

    float* data;
    this->vbo->Bind();
    GLCall(data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    /* Data layout (x, y, r, g, b) */
    for (int i = 0; i < count; i += stride) {
        data[i] = data[i] + xOffset;				// x
        data[i + 1] = data[i + 1] + yOffset;		// y
    }

    GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}
