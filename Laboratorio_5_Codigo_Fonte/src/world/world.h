#pragma once

#include "glm/glm.hpp"

#include "inttypes.h"
#include <string>
#include <vector>

class GLFWwindow;
class Terrain;
class Player;

class World{
private:
    //ambiente
    GLFWwindow *window;
    Terrain *terrain;
    glm::vec2 worldSize;

    //numero de itens que foram removidos do jogo
    int numGarbageItems;

    //player
    Player *player;

    //variáveis de controle
    bool gameDone;

    //matrizes
    glm::mat4 perspectiveProjection; //matriz perspectiva para o jogador
    glm::mat4 perspectiveView;

    //ortographic matrix for hud

    //métodos

    //inicializa o mundo baseado num arquivo
    void createWorld(std::string worldFile);
    //cria o jogador
    void createPlayer(GLFWwindow *window, glm::vec2 windowSize);

    //inicializa as matrizes de camera
    void preparePerspectiveCamera(glm::vec2 windowSize);
    //void prepareOrthoCamera(glm::vec2 windowSize);

    //usa a orientação do jogador para determinar para onde a camera deve apontar
    void controlCamera();



public:
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


    //permite o término do loop principal
    bool isGameDone();
};
