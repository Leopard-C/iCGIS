/***************************************************************
** MacroName:   GLCall
**
** description: 任何调用OpenGL的函数都写在宏GlCall中
**				GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
**				GLCall(glClear(GL_COLOR_BUFFER_BIT));
**
** last change: 2019-12-07
****************************************************************/
#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <GL/glew.h>


#define ASSERT(x) if (!(x)) __debugbreak()
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);


