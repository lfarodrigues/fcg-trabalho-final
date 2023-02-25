#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

class World;
class Shader;

class Terrain{
private:

    World *world; // para obter a dire��o do sol
    Shader *shader; // programa shader usado para renderiza��o do terreno

    //opengl
    GLuint vao;     // GL state usado quando terreno estiver sendo renderizado
    GLuint vbos[3];  // GL vertex buffer object para posi��o, coordenadas de textura, normais e indices de elemento

    //texturas
    //

    //propiedades do terreno
    int width;      // largura do terreno em tiles
    int length;     // comprimento do terreno em tiles
    float squareSize; // tamanho XZ do quadrado(tile)
    float *terrainHeights; // array de alturas

    //m�todos
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

    //intera��o
    float getHeight(glm::vec3 pos); // altura do terreno na posi��o passado como parametro

    //raycast
    //sombras


};
