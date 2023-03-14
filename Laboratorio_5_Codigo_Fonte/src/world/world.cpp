#include "world.h"
#include "terrain.h"
#include "../objects/player.h"
#include "../util/lodepng.h"
#include "stb_image.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <iostream>
#include <vector>
#include <string>

using namespace glm;

const glm::vec3 World::SUN_DIRECTION(glm::normalize(vec3(0.288, 1.2, 2.2)));

World::World(GLFWwindow *window, glm::vec2 windowSize, std::string worldFile)
{
    preparePerspectiveCamera(windowSize);
    //prepareOrthoCamera()

    createPlayer(window, windowSize);
    createWorld(worldFile);

    gameDone = 0;

    numGarbageItems = 0;
}

void World::preparePerspectiveCamera(glm::vec2 windowSize)
{
    const float FOV = 45.0f;
    const float ASPECT_RATIO = (float)windowSize.x / (float)windowSize.y;
    const float PERSPECTIVE_NEAR_RANGE = 0.1f;
    const float PERSPECTIVE_FAR_RANGE = 8000.0f;

    perspectiveProjection = glm::perspective(FOV, ASPECT_RATIO, PERSPECTIVE_NEAR_RANGE, PERSPECTIVE_FAR_RANGE);
}

//inicialização do mundo
void World::createWorld(std::string worldFile){
    //propiedades do terreno
    const float MAX_TERRAIN_HEIGHT = 800.0f;    // qual o ponto mais alto do terreno?
    const float TERRAIN_TILE_SIZE = 10.0f;      // cada quadrado do terreno possui 10m x 10m

    unsigned int width, length, channels;       // numero de quadrados que o mundo possui em largura e comprimento
    unsigned char *data;                        // dados lidos do arquivo de mundo
    unsigned char *dataPtr;
    unsigned char pixelData[4];

    float *heights;
    float *heighrPtr;

    unsigned int i, j;

    //constroi o terreno a partir de um arquivo
    lodepng_decode32_file(&data, &width, &length, worldFile.c_str());

    if(data!=NULL){
        dataPtr = data;
        heights = new float[width * length];
        heighrPtr = heights;

        worldSize = glm::vec2(width * TERRAIN_TILE_SIZE, length * TERRAIN_TILE_SIZE);

        for(i = 0; i < length; i++){
            for(j = 0; j < width; j++){
                //lê a cor do pixel atual
                memcpy(pixelData, dataPtr, sizeof(unsigned char) * 4);
                dataPtr += 4;

                //pixels vermelhos dizem a altura do terreno
                *heighrPtr++ = ((float)pixelData[0] / 255.0) * MAX_TERRAIN_HEIGHT;

                //TODO
                if(pixelData[1] == 255){
                    //pixels verdes indicam a presenca de uma arvore
                }
            }
        }

        //constroi o terreno agora que temos a altura de cada vertice
        terrain = new Terrain(this, width, length, TERRAIN_TILE_SIZE, heights);
        if(terrain==NULL){
            std::cout << "In World::createWorld() - Could not create terrain" << std::endl;
            exit(1);
        }
    }
    else{
        std::cout << "In World::createWorld() - Could not load world file" << std::endl;
        exit(1);
    }
    delete heights;
    delete[] data;
}

void World::createPlayer(GLFWwindow *window, glm::vec2 windowSize){
    const glm::vec3 STARTING_POS(2560.0, 0.0, -2560.0);
    player = new Player(window, this, STARTING_POS);
}

void World::controlCamera(){
    perspectiveView = glm::lookAt(player->getPos(), player->getCameraLook(), glm::vec3(0.0,1.0,0.0));
}

World::~World(){
    delete terrain;
    delete player;
}

void World::update(float dt){
    //remove qualquer coisa que não está mais sendo utilizada
    flushGarbage();

    //atualiza objetos
    player->update(dt);

    //atualiza posicao da camera e arma
    player->computeCameraOrientation();
    player->computeGunPosition();

    controlCamera();
}

void World::render(){
    glm::mat4 modelMat = glm::mat4(1.0);
    //renderiza tudo
    terrain->render(perspectiveProjection, perspectiveView, modelMat);
    player->render(perspectiveProjection, perspectiveView);
}

void World::addGarbageItem(){
    numGarbageItems++;
}

void World::flushGarbage(){
}

float World::getTerrainHeight(glm::vec3 pos){
    return terrain->getHeight(pos);
}

glm::vec3 World::getPlayerPos(){
    return player->getPos();
}

bool World::isGameDone(){
    return gameDone;
}
