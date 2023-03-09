#pragma once

#include "glad/glad.h"

#include <string>

#include "glm/glm.hpp"

//Classe para facilitar a criacao, bind e unbind de shaders
class Shader{
private:
    GLuint vertexShader;    //Id para o vertex shader
    GLuint fragmentShader; // Id para o fragment shader
    GLuint program;         //Id do programa

    GLint getUniLoc(GLuint, const char*); //
    void printShaderLogInfo(GLuint);        //imprime na tela informações da execução do shader
    char *textFileRead(const char*);        //usado para carregar o shader

public:
    static void unbind();           // "desliga" o shader removendo ele do contexto opengl

    //ctor
    Shader(std::string vertexFile, std::string fragmentFile);
    //dtor
    ~Shader();

    void bind();    // chamado uma vez para inserir o shader no contexto opengl
    void link();    // chamado uma vez após a atribuições por bindAttrib()

    void bindAttrib(const char* var, unsigned int index);

    // métodos para setar variaveis uniformes
	void uniform1i(const char*, int);
	void uniform1f(const char*, float);
	void uniform1fv(const char*, int, float*);
	void uniform2f(const char*, float, float);
	void uniform2fv(const char*, int, float*);
	void uniformVec2(const char*, glm::vec2);
	void uniform3iv(const char*, int, int*);
	void uniform3fv(const char*, int, float*);
	void uniform3f(const char*, const float, const float, const float);
	void uniformVec3(const char*, glm::vec3);
	void uniformMatrix3fv(const char*, int, GLfloat*, bool = false);
	void uniform4iv(const char*, int, int*);
	void uniform4fv(const char*, int, float*);
	void uniform4f(const char*, float, float, float, float);
	void uniformVec4(const char*, glm::vec4);
	void uniformMatrix4fv(const char*, int, GLfloat*, bool = false);
};
