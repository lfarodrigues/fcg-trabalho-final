#include "terrain.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <iostream>

Terrain::Terrain(World *world,int width, int length, float squareSize, float *heights){
    this->world = world;

    setupVBOs(width, length, squareSize, heights);
    //loadShader();
    //loadTextures();
}

Terrain::~Terrain(){
    delete[] terrainHeights;
    glDeleteBuffers(4, vbos);
    glDeleteVertexArrays(1, &vao);
    delete shader;
}

Terrain::setupVBOS(int width, int length, float squareSize, float *heights){
    const int NUM_VERTICES = width * length;
    const int NUM_INDICES = (width - 1) * (length - 1) * 6;
    const int NUM_TRIANGLES = NUM_INDICES / 3;

    //aloca memoria para vertices
    glm::vec3 *vertices = new glm::vec3[NUM_VERTICES];
    glm::vec3 *vertexPtr = vertices;
    GLfloat *terrainHeightPtr = heights;

    //tetxturas
    //glm::vec2 *baseTexCoords = new glm::vec2[NUM_INDICES];
    //glm::vec2 *baseTexCoordsPtr = baseTexCoords;

    //aloca memoria para indices
    GLuint *indices = new GLuint[NUM_INDICES];
    GLuint *indicesPtr = indices;

    //aloca memoria para normais
    glm::vec3 normals = new glm::vec3[NUM_VERTICES];
    short *shareCount = new short[NUM_INDICES];

    glm::vec3 v1, v2, v3;
	glm::vec3 va, vb;
	glm::vec3 normal;

	float average;
	int i, j, x, z
}

