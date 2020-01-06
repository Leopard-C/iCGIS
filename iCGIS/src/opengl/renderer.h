/*******************************************************
** class name:  Renderer
**
** description: 渲染器
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "opengl/vertexarray.h"
#include "opengl/indexbuffer.h"
#include "opengl/shader.h"
#include "opengl/texture.h"

class Renderer {
public:
	void Clear() const;
	void DrawPoint(const VertexArray* vao, const IndexBuffer* ibo, Shader* pointShader);
	void DrawLine(const VertexArray* vao, const IndexBuffer* ibo, Shader* lineShader);
	void DrawPolygon(const VertexArray* vao, const IndexBuffer* ibo, Shader* polygonShader);
	void DrawPolygonBorder(const VertexArray* vao, Shader* borderShader);
	void DrawHighlight(const VertexArray* vao, Shader* highlightShader);
	void DrawTexture(const VertexArray* vao, const IndexBuffer* ibo, const std::vector<Texture*>& texs, Shader* textureShader);
private:
};
