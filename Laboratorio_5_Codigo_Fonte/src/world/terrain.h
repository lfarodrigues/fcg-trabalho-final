#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

class World;
class Shader;

class Terrain{
private:

    World *world; // para obter a direção do sol
    Shader *shader; // programa shader usado para renderização do terreno

    //opengl
    GLuint vao;     // GL state usado quando terreno estiver sendo renderizado
    GLuint vbos[4];  // GL vertex buffer object para posição, coordenadas de textura, normais e indices de elemento

    //texturas
    GLuint region1Texture;				// the terrain uses and blends together several different textures based
	GLuint region2Texture;				// on the terrain height to give a somewhat-convincing illusion of realistic
	GLuint region3Texture;				// grassy hills and mountains
	GLuint shadowTexture;				// this texture is laid on top of the entire terrain object for low-res, static shadows

    //propiedades do terreno
    int width;      // largura do terreno em tiles
    int length;     // comprimento do terreno em tiles
    float squareSize; // tamanho XZ do quadrado(tile)
    float *terrainHeights; // array de alturas

    //métodos
    void setupVBOs(int width, int length, float squareSize, float *heights);
    void loadShader();

    void loadTextures();
public:
    //ctor
    Terrain(World *world,int width, int length, float squareSize, float *heights);

    //dtor
    ~Terrain();
    //renderiza todo o terreno
    void render(glm::mat4 &projection, glm::mat4 &view, glm::mat4 &model);

    //interação
    float getHeight(glm::vec3 pos); // altura do terreno na posição passado como parametro

    //raycast
    //sombras


};
