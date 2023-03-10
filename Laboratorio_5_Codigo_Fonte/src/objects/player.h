#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

class GLFWwindow;
class World;
class Shader;

class Player{
private:
    //estado
    typedef enum RELOADSTATE{
        STATE_LOADED,
        STATE_START_MOVING_DOWN,
        STATE_MOVING_DOWN,
        STATE_RELOADING,
        STATE_MOVING_UP
    }ReloadState;

    float deathImpactTimer;
    bool alive;
    //objetos do mundo importantes
    GLFWwindow *window;             // para entradas do mouse
    World *world;                   // para posicao do sol e elementos do mundo

    static const float MAX_LOOK_PITCH;

    //orientacao e movimentacao
    glm::vec3 pos;                  // posicao global do jogador
    glm::vec3 gunPos;               // posicao da arma relatica ao jogador
    glm::vec3 forward;              // vetor frente do jogador
    glm::vec3 side;                 // vetor do lado do jogador
    glm::vec3 up;                   //vetor de cima do jogador

    //fisica
    float gravity;
    bool touchingGround;
    bool isMoving;
    float jumpTimer;

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

    //arma
    float gunWalkBob;						// used to time the gun bob effect
	float gunWalkBobAmount;					// abs(sin(gunWalkBobAmount)) is used to compute gun walking bob effect

	bool triggerPressed;					// double-action gun---is the left mouse button currently down?
	bool gunRecoilFinished;					// has the recoil finished enough for us to allow another shot?

    float gunRecoilTimer;					// tempo da animcao de recoil
	float gunRecoilAmount;

	int numShotsInClip;						// numero atual de balas no cartucho antes de realizar reload

	ReloadState gunReloadState;				// current state of the gun reloading animation; see the ReloadState enum above
	float gunReloadTimer;					// timer em segundos para controlar a animacao de reload
	float gunReloadOffsetAmount;			// controle da animacao da arma

    //renderizacao da arma
	Shader *shader;							// shader program

	GLuint vao;								// vertex array object (i.e., GL state) para a arma
	GLuint vbos[3];							// vertex buffer objects (i.e., vertex, tex coords, normals) para a arma

	int numGunVertices;						// numero de vertices necessarios para a chamada de rendering
	GLuint gunDiffuseMap;					// textura da arma
	GLuint gunNormalMap;					// normal mapping usado para ilumincao per-fragment
	GLuint gunSpecularMap;					// textura especular para alterar a aparencia da arma
	GLuint gunEmissionMap;					// textura de emissao para os efeitos luminosos da arma (LEDs, etc)

    //carregamento de recursos
    void loadGun();
    void loadTextures();
    void loadShader();

    //rotinas de atualizacao
    void controlMouseInput(float dt);       // torna movimentacao do mouse em angulos da camera
    void computeWalkingVectors();           // computa os vetores usados para movimentacao
    void controlLooking(float dt);          // coloca limites nos angulos de visao do jogador
    void controlMoving(float dt);

public:
    static const float PLAYER_HEIGHT;

    Player(GLFWwindow *window, World *world, glm::vec3 pos);
    ~Player();

    //rotinas de atualizacao
    void update(float dt);
    void render(glm::mat4 &projection, glm::mat4 &view);

    //chamados externamente pelo mundo do jogo
    void computeCameraOrientation();

    //getters/setters
    glm::vec3 getPos();
    void setPos(glm::vec3 pos);

    glm::vec3 getCameraLook();
    glm::vec3 getCameraSide();
    glm::vec3 getCameraUp();

    bool getIsMoving();

    void die();
    bool isAlive();
};
