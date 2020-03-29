/*******************************************************
** namespace:   Env
**
** description:	Global variables
**
** last change: 2020-03-29
*******************************************************/
#ifndef ENV_H
#define ENV_H

#include "opengl/shader.h"
#include "opengl/renderer.h"
#include "operation/operationlist.h"

class GeoMap;

namespace Env {

/* Shaders */
extern Shader pointShader;
extern Shader lineShader;
extern Shader polygonShader;
extern Shader borderShader;
extern Shader highlightShader;
extern Shader textureShader;
extern Renderer renderer;
void createShaders();

/* Mouse Cursor */
enum class CursorType {
    Normal  = 0,
    Palm    = 1,
    Editing = 2,
    WhatIsThis = 3,
};

// Whether is editing map
extern bool isEditing;

// Mouse cursor type
// Normal, palm, etc.
extern CursorType cursorType;

// map
// There is only one map in one project
extern GeoMap* map;

// Project home path
extern QString HOME;

// Record the operations: move features, delete features, etc.
extern OperationList opList;

} // namespace Env

#endif // ENV_H
