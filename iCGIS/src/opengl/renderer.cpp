#include "renderer.h"
#include "glcall.h"

void Renderer::Clear() const
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
	GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
}

/* 渲染point */
void Renderer::DrawPoint(const VertexArray* vao, const IndexBuffer* ibo, Shader* pointShader)
{
	pointShader->Bind();
	vao->Bind();
	ibo->Bind();
	glPointSize(3);
	GLCall(glDrawElements(GL_POINTS, ibo->getCount(), GL_UNSIGNED_INT, nullptr));
	glPointSize(1);
}

/* 渲染line */
void Renderer::DrawLine(const VertexArray* vao, const IndexBuffer* ibo, Shader* lineShader)
{
	lineShader->Bind();
	vao->Bind();
	ibo->Bind();
	GLCall(glDrawElements(GL_LINE_STRIP, ibo->getCount(), GL_UNSIGNED_INT, nullptr));
}


/* 渲染polygon的内部填充色 */
void Renderer::DrawPolygon(const VertexArray* vao, const IndexBuffer* ibo, Shader* polygonShader)
{
	polygonShader->Bind();
	vao->Bind();
	ibo->Bind();

	// 绘制填充色
	GLCall(glDrawElements(GL_TRIANGLES, ibo->getCount(), GL_UNSIGNED_INT, nullptr));
}


/* 渲染polygon的边界 */
void Renderer::DrawPolygonBorder(const VertexArray* vao, Shader* borderShader)
{
	borderShader->Bind();
	vao->Bind();

	borderShader->SetUniform3f("u_borderColor", 0.0f, 0.0f, 0.0f);

	// 绘制边界
	int count = vao->getStridesCount();
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		GLCall(glDrawArrays(GL_LINE_STRIP, offset, vao->getStride(i)));
		offset += vao->getStride(i);
	}
}

/* 渲染高亮边界 */
void Renderer::DrawHighlight(const VertexArray* vao, Shader* highlightShader)
{
	highlightShader->Bind();
	vao->Bind();

	highlightShader->SetUniform3f("u_highlightColor", 0.0f, 1.0f, 1.0f);

	// 绘制边界
	int count = vao->getStridesCount();
	int offset = 0;
	glLineWidth(4);
	for (int i = 0; i < count; ++i) {
		GLCall(glDrawArrays(GL_LINE_STRIP, offset, vao->getStride(i)));
		offset += vao->getStride(i);
	}
	glLineWidth(1);
}


/* 贴图 */
void Renderer::DrawTexture(const VertexArray* vao, const IndexBuffer* ibo, const std::vector<Texture*>& texs, Shader* textureShader)
{
	//// 最多32个纹理
	//if (texs.size() > 32)
	//	return;

//	int texCount = texs.size();
//	for (int i = 0; i < texCount; ++i) {
//		glActiveTexture(GL_TEXTURE0 + i);
//		glBindTexture(GL_TEXTURE_2D, texs[i]->getID());
//	}

	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, texs[0]->getID()));

	textureShader->Bind();
	textureShader->SetUniform1i("ourTexture", 0);
	vao->Bind();
	ibo->Bind();
	GLCall(glDrawElements(GL_TRIANGLES, ibo->getCount(), GL_UNSIGNED_INT, nullptr));
}

