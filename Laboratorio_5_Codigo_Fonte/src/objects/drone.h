#pragma once

#include "object.h"

#include "glm/glm.hpp"

class World;

class Drone : public Object
{
private:
	static const float DEFAULT_HEALTH;					// quantidade de hits points até o drone cair

	World *world;										// handle para o acesso ao mundo do jogo e jogador

	glm::vec3 velocity;									// velocidade do drone em m/s
	glm::vec3 impactMotion;								// velocidade para impacto do tiro do jogador

	float distanceToPlayer;								// computada a cada frame
	bool playedWarning;									// modo de 'atencao' do drone

	float health;										// quantidade de hit points atual
	bool alive;											// hit points > 0?

	void moveTowardsPlayer(float dt);					// move o drone ate o jogador
	void controlImpactMotion(float dt);					// efeito de impacto apos ser atingido
	void controlGroundIntersection();					// previne interseccao com o terreno
    void controlDeath();								// detecta que o drone morreu

    void explode();										// boom

public:
	static const float MOVE_SPEED;						// velocidade do drone e constante

	Drone();
	Drone(World *world, glm::vec3 pos);
	~Drone();

    //update eh feito individualmente, mas os drones sao renderizados em conjunto atraves de DroneManager::render()
	void update(float dt);

	// verdadeiro se health > 0
	bool getAlive();

	// chamada se a colisao com um tiro for detectada
	void handleRayCollision(glm::vec3 dir, glm::vec3 point);
};
