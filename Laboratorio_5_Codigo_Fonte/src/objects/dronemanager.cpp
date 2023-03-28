#include "dronemanager.h"
#include "drone.h"

#include "../util/glmmodel.h"
#include "../util/shader.h"
#include "../util/loadtexture.h"

#include "../world/world.h"
#include "glad/glad.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
using namespace glm;

#include <cstdlib>
#include <iostream>
using namespace std;

DroneManager::DroneManager(World *world, int maxDrones)
{
	this -> world = world;
	this -> maxDrones = maxDrones;
	numDrones = 0;
	numDronesAlive = 0;

	drones = new Drone[maxDrones];
	modelMats = new mat4[maxDrones];

	loadModels();
	loadTextures();
	loadShader();
}

DroneManager::~DroneManager()
{
    glmDelete(droneBladeModel);
	glmDelete(droneBodyModel);

	glDeleteBuffers(4, bodyVBOs);
	glDeleteVertexArrays(1, &bodyVAO);
	delete bodyShader;

	glDeleteBuffers(4, bladesVBOs);
	glDeleteVertexArrays(1, &bladesVAO);
	delete bladesShader;

	delete[] drones;
	delete[] modelMats;
}

void DroneManager::loadModels()
{
	// le o modelo do corpo do drone
    droneBodyModel = glmReadOBJ((char*)"../../mesh/drone-body.obj");
    if(droneBodyModel)
    {
		glmScale(droneBodyModel, 1.0);
        //constroi os buffer objects e preenche com os dados de geometria carregados
		glGenVertexArrays(1, &bodyVAO);
		glBindVertexArray(bodyVAO);
		glGenBuffers(4, bodyVBOs);
		glmBuildVBO(droneBodyModel, &numBodyVertices, &bodyVAO, bodyVBOs);

		glBindBuffer(GL_ARRAY_BUFFER, bodyVBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * maxDrones, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)sizeof(vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 2));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 3));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	}

	// le o modelo das helices do drone
	droneBladeModel = glmReadOBJ((char*)"../../mesh/drone-blades.obj");
	if(droneBladeModel)
	{
		glmScale(droneBladeModel, 1.0);

		glGenVertexArrays(1, &bladesVAO);
		glBindVertexArray(bladesVAO);
		glGenBuffers(4, bladesVBOs);
		glmBuildVBO(droneBladeModel, &numBladesVertices, &bladesVAO, bladesVBOs);

		glBindBuffer(GL_ARRAY_BUFFER, bladesVBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * maxDrones, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)sizeof(vec4));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 2));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (GLvoid*)(sizeof(vec4) * 3));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
	}

	/*droneColliderModel = glmReadOBJ((char*)"../mesh/drone-collider.obj");
	if(droneColliderModel)
	{
		glmScale(droneColliderModel, 1.0);
	}*/
}

void DroneManager::loadTextures()
{
    diffuseMap = loadPNG("../../png/drone-diffuse-map.png");
    normalMap = loadPNG("../../png/drone-normal-map.png");
    specularMap = loadPNG("../../png/drone-specular-map.png");
    emissionMap = loadPNG("../../png/drone-emission-map.png");
    bladesTexture = loadPNG("../../png/drone-blades.png");
}

void DroneManager::loadShader()
{
	// note to self: the drone lighting is probably not correct (in terms of orientation) due
	// to the fact that we don't pass in the normal matrix in a per-instance manner like we
	// should (see DroneManager::render() for more on this)
	bodyShader = new Shader("../../shaders/solid-instanced.vert", "../../shaders/solid.frag");
	bodyShader -> bindAttrib("a_Vertex", 0);
	bodyShader -> bindAttrib("a_Normal", 1);
	bodyShader -> bindAttrib("a_TexCoord", 2);
	bodyShader -> bindAttrib("a_Model", 3);
	bodyShader -> link();
	bodyShader -> bind();
	bodyShader -> uniform1i("u_DiffuseMap", 0);
	bodyShader -> uniform1i("u_NormalMap", 1);
	bodyShader -> uniform1i("u_SpecularMap", 2);
	bodyShader -> uniform1i("u_EmissionMap", 3);
	bodyShader -> uniformVec3("u_Sun", World::SUN_DIRECTION);
	bodyShader -> uniformVec3("u_MaterialDiffuse", vec3(1.0, 1.0, 1.0));
	bodyShader -> uniformVec3("u_MaterialSpecular", vec3(1.0, 1.0, 1.0));
	bodyShader -> uniform1f("u_SpecularIntensity", 6.0);
	bodyShader -> uniform1f("u_SpecularHardness", 16.0);
	bodyShader -> uniform1f("u_NormalMapStrength", 2.0);
	bodyShader -> unbind();

	// shader simples para as helices
	bladesShader = new Shader("../../shaders/solid-instanced.vert", "../../shaders/plane.frag");
	bladesShader -> bindAttrib("a_Vertex", 0);
	bladesShader -> bindAttrib("a_Normal", 1);
	bladesShader -> bindAttrib("a_TexCoord", 2);
	bladesShader -> bindAttrib("a_Model", 3);
	bladesShader -> link();
	bladesShader -> bind();
	bladesShader -> uniform1i("u_Texture", 0);
	bladesShader -> uniformVec4("u_Color", vec4(0.0, 0.0, 0.0, 1.0));
	bladesShader -> unbind();
}

Drone *DroneManager::addDrone(vec3 pos)
{
	Drone *result = NULL;

	if(numDrones < maxDrones)
	{
		drones[numDrones] = Drone(world, pos);
		result = &drones[numDrones];

		numDrones ++;
	}
	else
	{
		cerr << "DroneManager::addDrone() cannot add a drone because the maximum of " << maxDrones << " has been reached" << endl;
		exit(1);
	}

	return result;
}

void DroneManager::update(float dt)
{
	Drone *curr = drones;

	mat4 *modelMatPtr;
	vec3 pos;
	vec3 newPos;
	int i;

	modelMatPtr = modelMats;
	numDronesAlive = 0;
	for(i = 0; i < numDrones; i ++)
	{
		if(curr -> getAlive())
		{
			curr -> update(dt);
			pos = curr -> getPos();

            //salva a matriz de modelagem para renderizar mais tarde
			curr -> getModelMat(modelMatPtr);
			modelMatPtr++;
			numDronesAlive ++;
		}
		curr ++;
	}

	// envia a geometria do corpo para a placa de video
	glBindVertexArray(bodyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bodyVBOs[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * numDrones, modelMats);

	// envia a geometria das helices para a placa de video
	glBindVertexArray(bladesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bladesVBOs[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mat4) * numDrones, modelMats);
}

void DroneManager::render(mat4 &projection, mat4 &view)
{
	// computa a matriz normal para iluminacao
	mat4 modelMatrix(1.0);
	mat3 normal = inverseTranspose(mat3(modelMatrix));

	// configura OpenGL para renderizar o corpo e as helices
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// renderiza as helices e o corpo do drone separadamente
	renderBodies(projection, view, normal);
	renderBlades(projection, view, normal);
}

void DroneManager::renderBodies(mat4 &projection, mat4 &view, mat3 &normal)
{
	bodyShader -> bind();
	bodyShader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	bodyShader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	bodyShader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

    // usamos mapas de texturas para iluminacao difusa, especular, de emissao e de normais para obter o efeito desejado ao renderizar
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, emissionMap);

	// instanced rendering
	glBindVertexArray(bodyVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, numBodyVertices, numDronesAlive);
}

void DroneManager::renderBlades(mat4 &projection, mat4 &view, mat3 &normal)
{
	bladesShader -> bind();
	bladesShader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	bladesShader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	//bladesShader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bladesTexture);

    //queremos as helices transparentes e nao sendo escrita no depth buffer
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// instanced rendering
	glBindVertexArray(bladesVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, numBladesVertices, numDronesAlive);

	// restauraa configuração padrão
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

bool DroneManager::isDroneCloseTo(vec3 pos, float distance)
{
	double DIST_SQUARED = distance * distance;

	Drone *curr = drones;
	vec3 dronePos;
	vec3 diff;
	double distSquared;
	bool result = false;
	int i = 0;

	while(!result && i < numDrones)
	{
		if(curr -> getAlive())
		{
			dronePos = curr -> getPos();
			distSquared = (dronePos.x - pos.x) * (dronePos.x - pos.x) +
						  (dronePos.y - pos.y) * (dronePos.y - pos.y) +
						  (dronePos.z - pos.z) * (dronePos.z - pos.z);
			result = distSquared < DIST_SQUARED;
		}
		curr ++;
		i ++;
	}

	return result;
}
