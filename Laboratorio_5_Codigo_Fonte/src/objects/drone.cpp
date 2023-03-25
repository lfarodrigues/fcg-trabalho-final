#include "drone.h"

#include "../world/world.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <iostream>

const float Drone::DEFAULT_HEALTH = 30.0;
const float Drone::MOVE_SPEED = 3.0;				// in meters per second

Drone::Drone() { }

Drone::Drone(World *world, glm::vec3 pos)
{
	this->world = world;
	setPos(pos);

	health = DEFAULT_HEALTH;
	alive = true;

	playedWarning = false;
}

Drone::~Drone()
{

}

void Drone::update(float dt)
{
	moveTowardsPlayer(dt);
	controlImpactMotion(dt);
	controlGroundIntersection();

	computeModelMat();

	controlDeath();
}

void Drone::moveTowardsPlayer(float dt)
{
	const float TILT_ANGLE = -M_PI / 16.0f;
	const float UPDATE_ORIENTATION_DISTANCE = 500.0;		// perto o suficiente para o player enxergar o drone

	const glm::vec3 DRONE_TARGET_ADJUSTMENT(0.0f, -0.3f, 0.0f);

	glm::vec3 pos = getPos();
	glm::vec3 player;
	glm::vec3 offset;

	glm::vec3 forward, up, side;

    // apenas movimentamos em direcao ao jogador se ainda estiver vivo, caso contrario movemos para frente
	if(world->isPlayerAlive())
	{
		// posicao do drone e posicao do jogador
		player = world->getPlayerPos() + DRONE_TARGET_ADJUSTMENT;

		// computa um vetor em direcao ao jogador
		offset = player - pos;
		distanceToPlayer = glm::length(offset);
		velocity = (offset / distanceToPlayer) * MOVE_SPEED * dt;

        // apenas atualiza a orientacao do drone se o jogador esta perto o suficiente para ve-lo
		if(distanceToPlayer < UPDATE_ORIENTATION_DISTANCE)
		{
			forward = glm::normalize(glm::vec3(velocity.x, 0.0f, velocity.z));
			up = glm::vec3(0.0, 1.0, 0.0);
			side = glm::cross(forward, up);
			up = glm::rotate(up, TILT_ANGLE, side);
			forward = glm::rotate(forward, TILT_ANGLE, side);

			// re-orienta em direcao ao jogador
			setUp(up);
			setSide(side);
			setForward(forward);
		}
    }

    // sempre movimenta para frente
    setPos(pos + velocity);
}

void Drone::controlImpactMotion(float dt)
{
	const float MOTION_DAMPENING = 0.9;

	impactMotion = impactMotion * MOTION_DAMPENING;
	setPos(getPos() + impactMotion * dt);
}

void Drone::controlGroundIntersection()
{
	const float LOW_ENOUGH_TO_CHECK = 2.0;
	const float MIN_ROTOR_HEIGHT = 0.5;

	glm::vec3 pos = getPos();
	float terrainHeight = world->getTerrainHeight(pos);
	float terrainHeightDiffs[4];
	float lowestHeight;

	// se estamos proximos ao terreno, entao verifica todas as quatro extremidades do drone
	if((terrainHeight - pos.y) < LOW_ENOUGH_TO_CHECK)
	{
		// altura onde cada um dos quatro motores está
		terrainHeightDiffs[0] = world->getTerrainHeight(glm::vec3(pos.x - 1.0, 0.0, pos.z - 1.0));
		terrainHeightDiffs[1] = world->getTerrainHeight(glm::vec3(pos.x - 1.0, 0.0, pos.z + 1.0));
		terrainHeightDiffs[2] = world->getTerrainHeight(glm::vec3(pos.x + 1.0, 0.0, pos.z - 1.0));
		terrainHeightDiffs[3] = world->getTerrainHeight(glm::vec3(pos.x + 1.0, 0.0, pos.z + 1.0));

		// obtem a menor
		lowestHeight = glm::min(glm::min(terrainHeightDiffs[0], terrainHeightDiffs[1]), glm::min(terrainHeightDiffs[2], terrainHeightDiffs[3]));

		// se está suficientemente baixo, entao levanta o drone um pouco
		if(pos.y < (lowestHeight + MIN_ROTOR_HEIGHT))
		{
			pos = glm::vec3(pos.x, pos.y + ((lowestHeight + MIN_ROTOR_HEIGHT) - pos.y), pos.z);
			setPos(pos);
		}
	}
}

void Drone::controlDeath()
{
	const float EXPLODE_REF_DIST = 25.0;
	const float EXPLODE_MAX_DIST = FLT_MAX;

	// detectou destruicao do drone
	if(health <= 0.0 && alive)
	{
		// grava isso
		alive = false;

		// explode!
		explode();

		// remove da lista de colliders
		flagAsGarbage();
		world->addGarbageItem();
	}
}

bool Drone::getAlive()
{
	return alive;
}

void Drone::handleRayCollision(glm::vec3 dir, glm::vec3 pos)
{
	const float RAY_COLLISION_DAMAGE = 11.0;

	impactMotion += dir * 15.0f;
	health -= RAY_COLLISION_DAMAGE;
}

void Drone::explode()
{

}
