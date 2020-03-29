#include "util/env.h"
#include "geo/map/geomap.h"

namespace Env {

Shader pointShader;
Shader lineShader;
Shader polygonShader;
Shader borderShader;
Shader highlightShader;
Shader textureShader;
Renderer renderer;

void createShaders() {
    printf("point");
    pointShader.create("res/shaders/point_vert.glsl", "res/shaders/point_frag.glsl");
    printf("line");
    lineShader.create("res/shaders/line_vert.glsl", "res/shaders/line_frag.glsl");
    printf("polygon");
    polygonShader.create("res/shaders/polygon_vert.glsl", "res/shaders/polygon_frag.glsl");
    printf("border");
    borderShader.create("res/shaders/border_vert.glsl", "res/shaders/border_frag.glsl");
    printf("hightlight");
    highlightShader.create("res/shaders/highlight_vert.glsl", "res/shaders/highlight_frag.glsl");
    printf("texture");
    textureShader.create("res/shaders/texture_vert.glsl", "res/shaders/texture_frag.glsl");
}

bool isEditing = false;
CursorType cursorType = CursorType::Normal;
GeoMap* map = new GeoMap();
OperationList opList;
QString HOME = ".";


} // namespace Env
