#pragma once

#include "glm/glm.hpp"
#include "../util/collisions.h"

typedef struct Sphere Sphere;
typedef struct LineSegment LineSegment;

class Object
{
private:
	glm::vec3 pos;							// posicao
	glm::vec3 forward;						// vetor de "frente"
	glm::vec3 side;							// vetor do lado
	glm::vec3 up;							// vetor de cima
	glm::mat4 modelMat;						// matriz de modelagem (sem incluir escala) composta dos 4 vetores acima

    Sphere *sphereCollider;
	bool garbage;							// podemos remover o objeto do jogo?

public:
    Object();
    virtual ~Object() = 0;									// deve criar classes derivadas

    void setPos(glm::vec3 pos);								// setter da posicao (esse e os outros 3 vetores formam a
    glm::vec3 getPos();										// matriz de modelagem

    void setForward(glm::vec3 forward);
    glm::vec3 getForward();

    void setSide(glm::vec3 side);
    glm::vec3 getSide();

    void setUp(glm::vec3 up);
    glm::vec3 getUp();

    bool flaggedAsGarbage();								// objeto esta marcado para remocao?
    void flagAsGarbage();									// marca o objeto para delecao

    void setModelMat(glm::mat4 &mat);						// manually sets the object's model matrix
    void computeModelMat();									// computa a matriz de modelagem baseado nos valores dos vetore pos, forward, side e up
    void getModelMat(glm::mat4 *mat);						// retorna uma copia da matriz de modelagem computada

    void setSphereCollider(glm::vec3 pos, float radius);
    Sphere* getSphereCollider();
    //determina se um objeto foi atingido por um raio e onde
	bool collidesWithRay(glm::vec3 &start, glm::vec3 &dir, float length);

    glm::vec3 BezierCurve(float t, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4);

    virtual void handleRayCollision(glm::vec3 dir, glm::vec3 point);

};
