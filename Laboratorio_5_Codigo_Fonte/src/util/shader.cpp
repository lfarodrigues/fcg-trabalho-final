#include "shader.h"

#include "glad/glad.h"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>

Shader::Shader(std::string vertex, std::string frag){
    GLint vertexCompiled;
    GLint fragmentCompiled;

    char *vertexFile = textFileRead(vertex.c_str());
    char *fragFile = textFileRead(frag.c_str());

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //especifica o codigo fonte para os dois shaders
    glShaderSource(vertexShader, 1, &vertexFile, NULL);
    glShaderSource(fragmentShader, 1, &fragFile, NULL);

    //compila o vertex shader
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS, &vertexCompiled);

    if(!vertexCompiled){
        std::cout << "Shader:Shader() could not compile vertex shader " << vertex << std::endl;
        printShaderLogInfo(vertexShader);
        exit(1);
    }

    //compila o fragment shader
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentCompiled);

    if(!fragmentCompiled){
        std::cout << "Shader::Shader() could not compile the fragment shader " << frag << std::endl;
        printShaderLogInfo(fragmentShader);
        exit(1);
    }

    if(vertexCompiled && fragmentCompiled){
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
    }
}

Shader::~Shader(){
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
}

void Shader::link(){
    GLint linked = false;

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if(!linked){
        std::cerr << "Shader::Shader() compiled but could not be linked" << std::endl;
        printShaderLogInfo(vertexShader);
        printShaderLogInfo(fragmentShader);
        exit(1);
    }
}

void Shader::bind(){
    glUseProgram(program);
}

void Shader::unbind(){
    glUseProgram(0);
}

void Shader::bindAttrib(const char* var, unsigned int index){
    glBindAttribLocation(program, index, var);
}

void Shader::uniform1f(const char *var, float val)
{
    glUniform1f(getUniLoc(program, var), val);
}

void Shader::uniform1i(const char *var, int val)
{
    glUniform1i(getUniLoc(program, var), val);
}

void Shader::uniform1fv(const char *var, int count, float *vals)
{
    glUniform1fv(getUniLoc(program, var), count, vals);
}

void Shader::uniform2f(const char *var, float v1, float v2)
{
    glUniform2f(getUniLoc(program, var), v1, v2);
}

void Shader::uniform2fv(const char *var, int count, float *vals)
{
    glUniform2fv(getUniLoc(program, var), count, vals);
}

void Shader::uniformVec2(const char *var, glm::vec2 v)
{
    uniform2f(var, v.x, v.y);
}

void Shader::uniform3iv(const char *var, int count, int *vals)
{
    glUniform3iv(getUniLoc(program, var), count, vals);
}

void Shader::uniform3fv(const char *var, int count, float *vals)
{
    glUniform3fv(getUniLoc(program, var), count, vals);
}

void Shader::uniform3f(const char *var, const float v1, const float v2, const float v3)
{
    glUniform3f(getUniLoc(program, var), v1, v2, v3);
}

void Shader::uniformVec3(const char *var, glm::vec3 v)
{
    uniform3f(var, v.x, v.y, v.z);
}

void Shader::uniformMatrix3fv(const char *var, GLsizei count, GLfloat *vals, bool transpose)
{
    glUniformMatrix3fv(getUniLoc(program, var), count, transpose, vals);
}

void Shader::uniform4iv(const char *var, int count, int *vals)
{
    glUniform4iv(getUniLoc(program, var), count, vals);
}

void Shader::uniform4fv(const char *var, int count, float *vals)
{
    glUniform4fv(getUniLoc(program, var), count, vals);
}

void Shader::uniform4f(const char *var, float v1, float v2, float v3, float v4)
{
    glUniform4f(getUniLoc(program, var), v1, v2, v3, v4);
}

void Shader::uniformVec4(const char *var, glm::vec4 v)
{
    uniform4f(var, v.x, v.y, v.z, v.w);
}

void Shader::uniformMatrix4fv(const char *var, GLsizei count, GLfloat *vals, bool transpose)
{
    glUniformMatrix4fv(getUniLoc(program, var), count, transpose, vals);
}

GLint Shader::getUniLoc(GLuint program, const char *name){
    GLint loc;

    loc = glGetUniformLocation(program, name);

    if(loc == -1){
        std::cerr << "Shader::getUniLoc(): uniform '" << name << "' has not been defined" << std::endl;
    }

    return loc;
}

void Shader::printShaderLogInfo(GLuint obj){
    int infoLogLength = 0;
    int charsWritten = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, & infoLogLength);

    if(infoLogLength > 1){
        infoLog = (char*)malloc(infoLogLength);
        glGetShaderInfoLog(obj, infoLogLength, &charsWritten, infoLog);
        std::cout << infoLog << std::endl;
        free(infoLog);
    }
}

char *Shader::textFileRead(const char *fn){
    FILE *fp;
    char *content = NULL;

    int count = 0;

    if(fn != NULL){
        fp = fopen(fn, "rt");
        if(fp != NULL){
            fseek(fp, 0, SEEK_END);
            rewind(fp);

            if(count > 0){
                content = (char*)malloc(sizeof(char) * (count+1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    else{
        std::cerr << "Shader::textFileRead() failed to open '" << fn << "'" << std::endl;
        exit(1);
    }

    return content;
}

