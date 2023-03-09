#include "player.h"
#include "../world/world.h"
#include "../util/shader.h"
#include "../util/loadmesh.h"

#include <iostream>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"


#define M_PI_2     1.57079632679489661923   // pi/2


const float Player::PLAYER_HEIGHT = 1.7; // o quanto a camera esta acima do chao

const float Player::MAX_LOOK_PITCH = M_PI_2 - 0.2;

Player::Player(GLFWwindow *window, World *world, glm::vec3 pos){
    this->window = window;
    this->world = world;
    this->pos = pos;

    jumpTimer = 0.0;
    gravity = 0.0f;
    isMoving = false;
    touchingGround = false;
    //
    targetLookAngleX = 0.0;
    targetLookAngleY = 0.0;
    lookAngleX = 0.0;
    lookAngleY = 0.0;

    alive = true;

    computeWalkingVectors();
    glfwGetCursorPos(window, &oldMouseX, &oldMouseY);
}

Player::~Player(){
}

void Player::loadGun(){

}

void Player::loadTextures(){

}

void Player::loadShader(){



}

void Player::update(float dt){
    if(isAlive()){
        controlMouseInput(dt);
        computeWalkingVectors();
        controlMoving(dt);
    }
    controlLooking(dt);
}

//TODO
void Player::render(glm::mat4 &projection, glm::mat4 &view){
    const glm::vec3 GUN_SIZE(-0.225, 0.225, 0.225);
	const float GUN_RECOIL_ROTATE_STRENGTH = -4.0;  // recuo da arma
	const float GUN_RELOAD_ROTATE_AMOUNT = M_PI_2;  // rotacao durante o recarregamento

    glm::mat4 gunMat;           // matriz de modelagem
    glm::mat4 viewLocalMat;     // matriz de vizualizacao
    glm::mat4 normalMat;        // inversa transposta da matriz de modelagem - usada pra calculo de iluminacao da arma

    //orienta a arma na mesma direcao da camera
    gunMat[0] = glm::vec4(cameraSide, 0.0);
    gunMat[1] = glm::vec4(cameraUp, 0.0);
    gunMat[2] = glm::vec4(cameraForward, 0.0);
    gunMat[3] = glm::vec4(gunPos, 1.0);

    //escalamento
    gunMat = glm::scale(gunMat, GUN_SIZE);
    gunMat = glm::rotate(gunMat, (gunRecoilAmount * GUN_RECOIL_ROTATE_STRENGTH) + (gunReloadOffsetAmount * GUN_RELOAD_ROTATE_AMOUNT), glm::vec3(1.0,0.0,0.0));

    // the gun is pretty small relative to the world, so it's probably a good idea to render it relative to the player
	// rather than relative to the world; this is really only necessary in really large environments where precision
	// becomes a problem for objects close up
	viewLocalMat = view;
	viewLocalMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

	// computa matriz normal para iluminacao
	normalMat = glm::inverseTranspose(glm::mat3(gunMat));

	// envia para GPU
	shader->bind();
	shader->uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader->uniformMatrix4fv("u_View", 1, value_ptr(viewLocalMat));
	shader->uniformMatrix4fv("u_Model", 1, value_ptr(gunMat));
	shader->uniformMatrix3fv("u_Normal", 1, value_ptr(normalMat));

	// usamos diffuse, normal, specular, and emission texture maps na renderizacao para produzir um efeito interessante
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gunDiffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gunNormalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gunSpecularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gunEmissionMap);

	// nao queremos a arma transparente
	glDisable(GL_BLEND);

	// renderiza
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, numGunVertices);
}

void Player::controlMouseInput(float dt){
    double mouseX, mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);

    targetLookAngleX -= (mouseY - oldMouseY) * 0.002;
    targetLookAngleY -= (mouseX - oldMouseX) * 0.002;
    if(targetLookAngleX > MAX_LOOK_PITCH) targetLookAngleX = MAX_LOOK_PITCH;
    if(targetLookAngleX < -MAX_LOOK_PITCH) targetLookAngleX = -MAX_LOOK_PITCH;

    oldMouseX = mouseX;
    oldMouseY = mouseY;
}

void Player::controlMoving(float dt) {
    const float MOVE_SPEED = 100.0;					// velocidade padrao 6.7 m/s

    const float GRAVITY_STRENGTH = 9.81;            // forca da gravidade
    const float JUMP_ACCEL_TIME = 0.10;				// aceleracao do pulo
	const float JUMP_STRENGTH = 60.0;				// aceleracao maxima do pulo

    glm::vec3 targetVelocity;					    // quao rapido queremos se mover

	isMoving = false;

	if(glfwGetKey(window, 'W') == GLFW_PRESS || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
	{
		targetVelocity += glm::vec3(0.0, 0.0, MOVE_SPEED);
		isMoving = true;
	}

	if(glfwGetKey(window, 'A') == GLFW_PRESS)
	{
		targetVelocity += glm::vec3(-MOVE_SPEED, 0.0, 0.0);
		isMoving = true;
	}

	if(glfwGetKey(window, 'D') == GLFW_PRESS)
	{
		targetVelocity += glm::vec3(MOVE_SPEED, 0.0, 0.0);
		isMoving = true;
	}

	if(glfwGetKey(window, 'S') == GLFW_PRESS)
	{
		targetVelocity += glm::vec3(0.0, 0.0, -MOVE_SPEED);
		isMoving = true;
	}

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && touchingGround)
	{
		jumpTimer = JUMP_ACCEL_TIME;
	}

	if(jumpTimer > 0.0)
	{
		gravity += (JUMP_STRENGTH * (jumpTimer / JUMP_ACCEL_TIME)) * dt;
	}
	jumpTimer -= dt;

	// adicionando gravidaade
	gravity -= GRAVITY_STRENGTH * dt;
	pos.y += gravity * dt;

	pos = pos + (forward * targetVelocity.z * dt) + (side * targetVelocity.x * dt);

    //verifica colisao com o terreno
    if(pos.y < world->getTerrainHeight(pos) + PLAYER_HEIGHT){
        pos.y = world->getTerrainHeight(pos) + PLAYER_HEIGHT;
        gravity = 0.0;
        touchingGround = true;
    }
    else {
        touchingGround = false;
    }
}

void Player::controlLooking(float dt){
    lookAngleX = lookAngleX + (targetLookAngleX - lookAngleX) * 0.8;
    if(lookAngleX > MAX_LOOK_PITCH) lookAngleX = MAX_LOOK_PITCH;
    if(lookAngleX < -MAX_LOOK_PITCH) lookAngleX = -MAX_LOOK_PITCH;

    lookAngleY = lookAngleY + (targetLookAngleY - lookAngleY) * 0.8;
}

void Player::computeCameraOrientation(){
    glm::mat4 camera = glm::mat4(1.0);

    camera = glm::rotate(camera, lookAngleY, glm::vec3(0.0,1.0,0.0));
    camera = glm::rotate(camera, lookAngleX, glm::vec3(1.0,0.0,0.0));

    cameraForward = -glm::vec3(camera[2]);
    cameraSide = glm::vec3(camera[0]);
    cameraUp = glm::vec3(camera[1]);
    cameraLook = pos + cameraForward;
}

void Player::computeWalkingVectors(){
    up = glm::vec3(0.0,1.0,0.0);
    forward = glm::rotate(glm::vec3(0.0,0.0,-1.0), lookAngleY, up);
    side = glm::normalize(glm::cross(forward, up));
}

glm::vec3 Player::getPos(){
    return pos;
}
void Player::setPos(glm::vec3 pos){
    this->pos = pos;
}

glm::vec3 Player::getCameraLook(){
    return cameraLook;
}
glm::vec3 Player::getCameraSide(){
    return cameraSide;
}
glm::vec3 Player::getCameraUp(){
    return cameraUp;
}

bool Player::getIsMoving(){
    return isMoving;
}
void Player::die()
{

	if(alive)
	{
		alive = false;
		deathImpactTimer = 0.0;
	}
}

bool Player::isAlive()
{
	return alive;
}
