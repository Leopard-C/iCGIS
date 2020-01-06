#include "opengl/glcall.h"
#include "logger.h"

#include <string>

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError()) {
		char errorMsg[512] = { 0 };
		sprintf(errorMsg, "[OpenGL Error] (0x%04x) %s %s %d", error, function, file, line);
		printf("%s\n", errorMsg);
		LError(errorMsg);
		return false;
	}
	return true;
}

