#pragma once

#include "glm/glm.hpp"

class Object
{
private:
	glm::vec3 pos;							// posicao
	glm::vec3 forward;						// vetor de "frente"
	glm::vec3 side;							// vetor do lado
	glm::vec3 up;							// vetor de cima
	glm::mat4 modelMat;						// matriz de modelagem (sem incluir escala) composta dos 4 vetores acima

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
};
