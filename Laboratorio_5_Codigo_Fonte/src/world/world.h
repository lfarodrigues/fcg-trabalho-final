#pragma once

#include "glm/glm.hpp"

#include "inttypes.h"
#include <string>
#include <vector>

class GLFWwindow;
class Terrain;
class Player;
class DroneManager;
class Object;
class Cow;

class World{
private:
    //ambiente
    GLFWwindow *window;
    Terrain *terrain;
    glm::vec2 worldSize;
	static const float BULLET_RANGE;    // quao longe deve viajar a bala

    //numero de itens que foram removidos do jogo
    int numGarbageItems;

    //player
    Player *player;

    //vaca
    Cow *cow;

    //drones
    DroneManager *drones;
    //variáveis de controle
    bool gameDone;

    std::vector<Object*> rayCollidables; // lista de objetos que podem ser atirados pelo jogador


    //matrizes
    glm::mat4 perspectiveProjection; //matriz perspectiva para o jogador
    glm::mat4 perspectiveView;

    //métodos
    //colisoes
    bool getTerrainCollision(glm::vec3 bulletStart, glm::vec3 bulletDir, glm::vec3 &intersect, float &distance);
	Object *getObjectsCollision(glm::vec3 bulletStart, glm::vec3 bulletDir, glm::vec3 &intersect, float &distance);

    //insere um drone no mundo
    void addDrone(glm::vec3 pos);

    //inicializa o mundo baseado num arquivo
    void createWorld(std::string worldFile);
    //cria o jogador
    void createPlayer(GLFWwindow *window, glm::vec2 windowSize);

    //inicializa as matrizes de camera
    void preparePerspectiveCamera(glm::vec2 windowSize);
    //void prepareOrthoCamera(glm::vec2 windowSize);

    //usa a orientação do jogador para determinar para onde a camera deve apontar
    void controlCamera();

    //interacao do jogador com outros objetos
    void controlPlayerDeath();

public:
    static const glm::vec3 SUN_DIRECTION;
    //ctor
    World(GLFWwindow *window, glm::vec2 windowSize, std::string worldFile);

    //dtor
    ~World();

    //atualização de estado e renderizacao
    void update(float dt);
    void render();
    void addGarbageItem();

    bool isPlayerAlive();
    void flushGarbage();
    glm::vec3 getPlayerPos();
    //
    float getTerrainHeight(glm::vec3 pos);

    //
    void fireBullet(glm::vec3 bulletStart, glm::vec3 bulletDir);

    //permite o término do loop principal
    bool isGameDone();
};
