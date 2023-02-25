#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

class GLFWwindow;
class World;
class Shader;

class Player{
private:
    //objetos do mundo importantes
    GLFWwindow *window;             // para entradas do mouse
    World *world;                   // para posicao do sol e elementos do mundo

    static const float MAX_LOOK_PITCH;

    //orientacao e movimentacao
    glm::vec3 pos;                  // posicao global do jogador
    glm::vec3 forward;              // vetor frente do jogador
    glm::vec3 side;                 // vetor do lado do jogador
    glm::vec3 up;                   //vetor de cima do jogador

    //fisica
    float gravity;
    bool touchingGround;
    bool isMoving;

    //mouse
    double oldMouseX;                           // usado para movimentacao do mouse
    double oldMouseY;

    float targetLookAngleX;
    float targetLookAngleY;

    float lookAngleX;
    float lookAngleY;

    glm::vec3 cameraForward;                     // direcao da frente da camera
    glm::vec3 cameraSide;
    glm::vec3 cameraUp;
    glm::vec3 cameraLook;

    //rotinas de atualizacao
    void controlMouseInput(float dt);           // torna movimentacao do mouse em angulos da camera
    void computeWalkingVectors();       // computa os vetores usados para movimentacao
    void controlLooking(float dt);               // coloca limites nos angulos de visao do jogador

public:
    static const float PLAYER_HEIGHT;

    Player(GLFWwindow *window, World *world, glm::vec3 pos);
    ~Player();

    //rotinas de atualizacao
    void update(float dt);
    void render(float dt);

    //chamados externamente pelo mundo do jogo
    void computeCameraOrientation();

    //getters/setters
    glm::vec3 getPos();
    void setPos(glm::vec3 pos);

    glm::vec3 getCameraLook();
    glm::vec3 getCameraSide();
    glm::vec3 getCameraUp();

    bool getIsMoving();
};
