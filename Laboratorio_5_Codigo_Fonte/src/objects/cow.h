#pragma once
#include "object.h"
#include "glm/glm.hpp"
#include "glad/glad.h"

typedef struct _GLMmodel GLMmodel;

class World;
class Shader;

class Cow : Object{
private:
    GLMmodel *geometry;                     // geometria
    //renderizacao da vaca
    GLuint vao;								// vertex array object (i.e., GL state) para a vaca
	GLuint vbos[2];							// vertex buffer objects (i.e., vertex, tex coords, normals) para a vaca

    glm::mat4 modelMat;

    World *world;

	Shader *shader;							// shader program
	int numCowVertices;
public:
    Cow(World *world);
    ~Cow();
    void loadModel();
    void loadShader();

    void render(glm::mat4 &projection, glm::mat4 &view);
    void update(float dt);
};
