#pragma once
#include "glm/glm.hpp"

#include "inttypes.h"
#include <string>
#include <vector>

class Terrain;
class Player;

class World{
private:
    //ambiente
    GLFWwindow *window;
    Terrain *terrain;
    glm::vec2 worldSize;

    //player
    Player *player;

    //vari�veis de controle
    bool gameDone;

    //matrizes
    glm::mat4 perspectiveMatrix; //matriz perspectiva para o jogador
    glm::mat4 perspectiveView;

    //ortographic matrix for hud

    //m�todos

    //inicializa o mundo baseado num arquivo
    void createWorld(std::string worldFile);
    //cria o jogador
    void createPlayer(GLFWwindow *window, glm::vec2 windowSize);

    //inicializa as matrizes de camera
    void preparePerspectiveCamera(glm::vec2 windowSize);
    //void prepareOrthoCamera(glm::vec2 windowSize);

    //usa a orienta��o do jogador para determinar para onde a camera deve apontar
    void controlCamera();



public:
    //ctor
    World(GLFWwindow *window, glm::vec2 windowSize, std::string worldFile);

    //atualiza��o de estado e renderizacao
    void update(float dt);
    void render();

    glm::vec3 getPlayerPos();
    bool isPlayerAlive();

    //permite o t�rmino do loop principal
    bool isGameDone();
};
