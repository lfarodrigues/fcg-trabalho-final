#include <iostream>
#include "object.h"
#include "../util/collisions.h"
#include "glm/glm.hpp"
using namespace glm;

Object::Object()
{
	forward = vec3(0.0, 0.0, 1.0);
	side = vec3(1.0, 0.0, 0.0);
	up = vec3(0.0, 1.0, 0.0);
	computeModelMat();
    sphereCollider = NULL;
	garbage = false;
}

Object::~Object()
{
	if(sphereCollider != NULL)
	{
		delete sphereCollider;
	}
}

void Object::setPos(vec3 pos) { this -> pos = pos; }
vec3 Object::getPos() { return pos; }

void Object::setForward(vec3 forward) { this -> forward = forward; }
vec3 Object::getForward() { return forward; }

void Object::setSide(vec3 side) { this -> side = side; }
vec3 Object::getSide() { return side; }

void Object::setUp(vec3 up) { this -> up = up; }
vec3 Object::getUp() { return up; }

bool Object::flaggedAsGarbage() { return garbage; }
void Object::flagAsGarbage() { garbage = true; }

void Object::setModelMat(mat4 &modelMat)
{
	this -> modelMat = modelMat;
}

void Object::computeModelMat()
{
	// update our model matrix based on our direction vectors
	modelMat[0] = vec4(side, 0.0f);
	modelMat[1] = vec4(up, 0.0f);
	modelMat[2] = vec4(forward, 0.0f);
	modelMat[3] = vec4(pos, 1.0f);
}

void Object::getModelMat(mat4 *modelMat)
{
	*modelMat = this -> modelMat;
}

bool Object::collidesWithRay(vec3 &start, vec3 &dir, float length)
{
    float out;
    LineSegment lineSegment(start, start+length);
	if (sphereCollider && Intersect(lineSegment, *sphereCollider, out)){
        //printf("\n%f\n", out);
        return true;
	}
    return false;
}

void Object::setSphereCollider(glm::vec3 pos, float radius)
{
    if(sphereCollider == NULL){
        sphereCollider = new Sphere(pos, radius);
    }
    else{
        sphereCollider->mCenter = pos;
        sphereCollider->mRadius = radius;
    }
}

Sphere* Object::getSphereCollider(){
    return sphereCollider;
}

glm::vec3 Object::BezierCurve(float t, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
{
    glm::vec3 interp;

    interp.x = pow((1 - t), 3)*p1.x + 3*t*pow((1 - t), 2)*p2.x + 3*pow(t, 2)*(1 - t)*p3.x + pow(t, 3)*p4.x;
    interp.y = pow((1 - t), 3)*p1.y + 3*t*pow((1 - t), 2)*p2.y + 3*pow(t, 2)*(1 - t)*p3.y + pow(t, 3)*p4.y;
    interp.z = pow((1 - t), 3)*p1.z + 3*t*pow((1 - t), 2)*p2.z + 3*pow(t, 2)*(1 - t)*p3.z + pow(t, 3)*p4.z;

    return interp;
}


void Object::handleRayCollision(vec3 dir, vec3 point) { }


