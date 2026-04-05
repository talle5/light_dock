#ifndef UNITY_SHELL_WAYLAND_SHADER_H
#define UNITY_SHELL_WAYLAND_SHADER_H
#include <GLES3/gl3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <array>

class Shader
{
public:
    GLuint ID{0};

    void LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath)
    {
        const std::string base_path = "/home/talles/Documentos/code/c++/unity/shaders/";
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            vShaderFile.open(base_path + vertexPath);
            fShaderFile.open(base_path + fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "⚠️ | ERRO::SHADER: Ficheiro não encontrado ou inacessível!\n"
                << "Caminho V: " << vertexPath << "\n"
                << "Caminho F: " << fragmentPath << std::endl;
            return;
        }

        Compile(vertexCode, fragmentCode);
    }

    void Compile(const std::string& vertexSource, const std::string& fragmentSource)
    {

        const char* vCode = vertexSource.c_str();
        const char* fCode = fragmentSource.c_str();

        // Vertex Shader
        GLuint sVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(sVertex, 1, &vCode, nullptr);
        glCompileShader(sVertex);
        CheckCompileErrors(sVertex, "VERTEX");

        // Fragment Shader
        GLuint sFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sFragment, 1, &fCode, nullptr);
        glCompileShader(sFragment);
        CheckCompileErrors(sFragment, "FRAGMENT");

        // Shader Program
        this->ID = glCreateProgram();
        glAttachShader(this->ID, sVertex);
        glAttachShader(this->ID, sFragment);
        glLinkProgram(this->ID);
        CheckCompileErrors(this->ID, "PROGRAM");

        glDeleteShader(sVertex);
        glDeleteShader(sFragment);
    }

    Shader& Use()
    {
        glUseProgram(this->ID);
        return *this;
    }

    void SetMatrix4(const char* name, const float* matrix) const
    {
        glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, matrix);
    }

    void SetVector4(const char* name, float x, float y, float z, float w) const
    {
        glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
    }

    void SetVector2(const char* name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(this->ID, name), x, y);
    }

    void SetFloat(const char* name, float value) const
    {
        glUniform1f(glGetUniformLocation(this->ID, name), value);
    }

    void SetInteger(const char* name, int value) const
    {
        glUniform1i(glGetUniformLocation(this->ID, name), value);
    }

private:
    static void CheckCompileErrors(GLuint object, std::string type)
    {
        int success;

        std::array<char,1024> infoLog = {};

        if (type != "PROGRAM")
        {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(object, 1024, nullptr, infoLog.data());
                std::cout << "| ERRO::SHADER: Compilação falhou: " << type << "\n"
                    << infoLog.data() << "\n -- --------------------------------------------------- -- "
                    << std::endl;
            }
        }
        else
        {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(object, 1024, nullptr, infoLog.data());
                std::cout << "| ERRO::SHADER: Linkagem falhou: " << type << "\n"
                    << infoLog.data() << "\n -- --------------------------------------------------- -- "
                    << std::endl;
            }
        }
    }
};
#endif
