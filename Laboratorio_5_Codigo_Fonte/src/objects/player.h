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
    float deathImpactAmount;

    bool alive;
    //objetos do mundo importantes
    GLFWwindow *window;             // para entradas do mouse
    World *world;                   // para posicao do sol e elementos do mundo

	static const float GUN_RECOIL_ANIM_TIME;		// length of the gun recoil animation in seconds
	static const float DEATH_IMPACT_ANIM_TIME;		// length of the player impact animation in seconds

	static const float MAX_LOOK_PITCH;				// min and max pitch angle for the camera in radians


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
    bool useLookAt;
    float dtAcum;

    //arma
    float gunWalkBob;						// used to time the gun bob effect
	float gunWalkBobAmount;					// abs(sin(gunWalkBobAmount)) is used to compute gun walking bob effect

	bool triggerPressed;					// double-action gun---is the left mouse button currently down?
	bool gunRecoilFinished;					// has the recoil finished enough for us to allow another shot?

    float gunRecoilTimer;					// tempo da animcao de recoil
	float gunRecoilAmount;

	int numShotsInClip;						// numero atual de balas no cartucho antes de realizar reload

	ReloadState gunReloadState;				// estado atual da animacao de recarregamento; see the ReloadState enum above
	float gunReloadTimer;					// timer em segundos para controlar a animacao de reload
	float gunReloadOffsetAmount;			// controle da animacao da arma

    //renderizacao da arma
	Shader *shader;							// shader program

	GLuint vao;								// vertex array object (i.e., GL state) para a arma
	GLuint vbos[4];							// vertex buffer objects (i.e., vertex, tex coords, normals) para a arma

	int numGunVertices;						// numero de vertices necessarios para a chamada de rendering
	GLuint gunDiffuseMap;					// textura da arma
	GLuint gunNormalMap;					// normal mapping usado para ilumincao per-fragment
	GLuint gunSpecularMap;					// textura especular para alterar a aparencia da arma
	GLuint gunEmissionMap;					// textura de emissao para os efeitos luminosos da arma (LEDs, etc)

    //carregamento de recursos
    void loadGunAndSetupVBOs();
    void loadTextures();
    void loadShader();

    //rotinas de atualizacao
    void controlMouseInput(float dt);       // torna movimentacao do mouse em angulos da camera
    void computeWalkingVectors();           // computa os vetores usados para movimentacao
    void controlLooking(float dt);          // coloca limites nos angulos de visao do jogador
    void controlMoving(float dt);
    void controlGunBobbing(float dt);
public:
	static const float PLAYER_HEIGHT;		// quao alto a camera está do chão

	static const int MAX_ROUNDS_PER_CLIP;	// quantas baladas temos antes de precisar recarregar

    Player(GLFWwindow *window, World *world, glm::vec3 pos);
    ~Player();

    //rotinas de atualizacao
    void update(float dt);
    void render(glm::mat4 &projection, glm::mat4 &view);

    //chamados externamente pelo mundo do jogo
    void computeCameraOrientation();
    void computeGunPosition();
    void controlGunRecoil(float dt);
    void controlGunReloading(float dt);

    //getters/setters
    glm::vec3 getPos();
    void setPos(glm::vec3 pos);

    glm::vec3 getCameraLook();
    glm::vec3 getCameraSide();
    glm::vec3 getCameraUp();

    bool getIsMoving();
    bool getUseLookAt();
    void die();
    bool isAlive();
};
