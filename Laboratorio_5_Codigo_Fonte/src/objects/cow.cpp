#include <iostream>
#include "cow.h"
#include "../world/world.h"
#include "../util/shader.h"
#include "../util/glmmodel.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

Cow::Cow(World *world)
{
	this -> world = world;

	loadModel();
	loadShader();

	modelMat = glm::mat4(1.0);
}

Cow::~Cow(){
    glmDelete(geometry);

	glDeleteBuffers(4, vbos);
	glDeleteVertexArrays(1, &vao);
	delete shader;
}
void Cow::loadModel(){
    geometry = glmReadOBJ((char*)"../../mesh/cow.obj");
    if(geometry)
    {
		glmScale(geometry, 1.0);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbos);
		glmBuildVBO(geometry, &numCowVertices, &vao, vbos);
	} else {
        std::cout << "In Cow::loadModel(): Could not load the model" << std::endl;
	}

    // envia a geometria do corpo para a placa de video
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float), &modelMat);
}

void Cow::loadShader(){
    shader = new Shader("../../shaders/solid-cow.vert","../../shaders/solid-cow.frag");
    shader->bindAttrib("a_Vertex", 0);
    shader->link();
    shader->bind();
    shader->unbind();
}

void Cow::render(glm::mat4 &projection, glm::mat4 &view){
    //glm::vec3 normalMat = glm::inverseTranspose(modelMat);

	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(modelMat));

	// renderiza
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, numCowVertices);
}

void Cow::update(float dt){

}
