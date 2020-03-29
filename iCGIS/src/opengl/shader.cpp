#include "shader.h"
#include "glcall.h"
#include "util/logger.h"

#include <iostream>
#include <fstream>
#include <sstream>

Shader::~Shader()
{
    GLCall(glDeleteProgram(rendererID));
}

void Shader::create(const std::string vsPath, const std::string fsPath)
{
    std::string vsSource = parseShader(vsPath);
    std::string fsSource = parseShader(fsPath);
    rendererID = createShader(vsSource, fsSource);
}

void Shader::Bind() const
{
    GLCall(glUseProgram(rendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

std::string Shader::parseShader(const std::string& filepath)
{
    std::string sourceCode;
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        ifs.open(filepath);
        std::stringstream iss;
        iss << ifs.rdbuf();
        ifs.close();
        sourceCode = iss.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    return sourceCode;
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id,1,&src,nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)calloc(length, 1);
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
            "shader" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    GLCall(unsigned int program = glCreateProgram());
    GLCall(unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader));
    GLCall(unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader));

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (uniformLocationCache.find(name)!= uniformLocationCache.end())
        return uniformLocationCache[name];

    GLCall(int location = glGetUniformLocation(rendererID, name.c_str()));
    if (location == -1 ) {
        std::string errmsg = "Warning: uniform '";
        errmsg += name;
        errmsg += "' doesn't exist!";
        std::cout << errmsg << std::endl;
        LError(errmsg);
    }
    uniformLocationCache[name]= location;
    return location;
}
