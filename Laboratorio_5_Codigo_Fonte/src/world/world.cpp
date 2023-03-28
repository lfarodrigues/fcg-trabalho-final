#include "world.h"
#include "terrain.h"
#include "../objects/player.h"
#include "../objects/drone.h"
#include "../objects/dronemanager.h"
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

#define M_PI       3.14159265358979323846

using namespace glm;

const glm::vec3 World::SUN_DIRECTION(glm::normalize(vec3(0.288, 1.2, 2.2)));
const float World::BULLET_RANGE = 500.0;    // quao longe a bala deve ir

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

    //drones
    const float DRONE_MIN_DIST = 50.0;
	const int NUM_DRONES = 180;					// quantos drones para inserir no mundo

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

        //cria os drones
        drones = new DroneManager(this, NUM_DRONES);
        for(i = 0; i < NUM_DRONES; i ++)
        {
            float angle = linearRand(-M_PI, M_PI);
            float dist = linearRand(DRONE_MIN_DIST, worldSize.x / 2.0f);
            addDrone(player->getPos() + glm::vec3(sin(angle) * dist, 0.0, cos(angle) * dist));
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
    delete drones;
}

void World::update(float dt){
    //remove qualquer coisa que não está mais sendo utilizada
    flushGarbage();

    //atualiza objetos
    player->update(dt);
    drones->update(dt);
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
    drones->render(perspectiveProjection, perspectiveView);
}

void World::addDrone(glm::vec3 pos)
{
	Drone *drone;

	// alinha o drone com o chao
	pos.y = getTerrainHeight(pos) + 10.0;

	// adiciona o drone ao sistema
	drone = drones -> addDrone(pos);
}

void World::fireBullet(glm::vec3 bulletStart, glm::vec3 bulletDir){
 	const float BULLET_RANGE_SQUARED = BULLET_RANGE * BULLET_RANGE;

	const float BULLET_CYLINDER = 25.0;
	const float BULLET_CYLINDER_SQUARED = BULLET_CYLINDER * BULLET_CYLINDER;

	glm::vec3 bulletEnd = bulletStart + bulletDir * BULLET_RANGE;

	// update the colliders for any objects that might be hit according to a quick cylinder test
	//updateRayCollidablesInCylinder(bulletStart, bulletEnd, BULLET_RANGE_SQUARED, BULLET_CYLINDER_SQUARED);

	glm::vec3 terrainIntersect;
	float terrainDistance;

	glm::vec3 objectIntersect;
	float objectDistance;

	glm::vec3 impactPoint;

	//bool terrainCollision = getTerrainCollision(bulletStart, bulletDir, terrainIntersect, terrainDistance);
	//Object *objectCollision = getObjectsCollision(bulletStart, bulletDir, objectIntersect, objectDistance);

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

bool World::isPlayerAlive()
{
	return player -> isAlive();
}
