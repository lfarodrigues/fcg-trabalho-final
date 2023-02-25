#include "player.h"
#include "../world/world.h"

#include <iostream>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"


#define M_PI_2     1.57079632679489661923   // pi/2


const float Player::PLAYER_HEIGHT = 1.7; // o quanto a camera esta acima do chao

const float Player::MAX_LOOK_PITCH = M_PI_2 - 0.2;

Player::Player(GLFWwindow *window, World *world, glm::vec3 pos){
    this->window = window;
    this->world = world;
    this->pos = pos;

    gravity = 0.0f;
    isMoving = false;
    touchingGround = false;
    //
    targetLookAngleX = 0.0;
    targetLookAngleY = 0.0;
    lookAngleX = 0.0;
    lookAngleY = 0.0;

    computeWalkingVectors();
    glfwGetCursorPos(window, &oldMouseX, &oldMouseY);
}

Player::~Player(){
}

void Player::update(float dt){
    controlMouseInput(dt);
    computeWalkingVectors();
    controlLooking(dt);
}

//TODO
void Player::render(float dt){

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
