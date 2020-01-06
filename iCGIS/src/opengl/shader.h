/*****************************************************************************
** class name:  Shader
**
** description: 着色器，创建本类对象时指定几何着色器和片元着色器的GLSL语言代码
**				要使用一个shader绘图，执行调用Bind()函数即可
**
** last change: 2020-01-02
*****************************************************************************/
#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
	Shader(const std::string vsPath, const std::string fsPath);
	~Shader();
	void Bind() const;
	void Unbind() const;

	//Set uniforms
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3 );
	void SetUniformMat4f(const std::string& name, const glm::mat4&  matrix);

private:
	std::string parseShader (const std::string& filepath);
	unsigned int compileShader (unsigned int type, const std::string& source);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);
	int GetUniformLocation (const std::string& name);
private:
	unsigned int rendererID;

	// uniform变量的的位置缓存
	// 每次读取uniform变量比较耗时，缓存下来可以加快以后的读取速度
	std::unordered_map<std::string, int> uniformLocationCache;
};

