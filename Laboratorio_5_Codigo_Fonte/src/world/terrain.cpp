#include "terrain.h"
#include "../util/shader.h"
#include "world.h"
#include "../util/loadtexture.h"

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <iostream>

using namespace glm;

Terrain::Terrain(World *world,int width, int length, float squareSize, float *heights){
    this->world = world;

    setupVBOs(width, length, squareSize, heights);
    loadShader();
    loadTextures();
}

Terrain::~Terrain(){
    delete[] terrainHeights;
    glDeleteBuffers(4, vbos);
    glDeleteVertexArrays(1, &vao);
    delete shader;
}

void Terrain::setupVBOs(int width, int length, float squareSize, float *heights){
	const int NUM_VERTICES = width * length;
	const int NUM_INDICES = (width - 1) * (length - 1) * 6;
	const int NUM_TRIANGLES = NUM_INDICES / 3;

	vec3 *vertices = new vec3[NUM_VERTICES];
	vec3 *vertexPtr = vertices;
	GLfloat *terrainHeightPtr;

	vec2 *baseTexCoords = new vec2[NUM_VERTICES];
	vec2 *baseTexCoordPtr = baseTexCoords;

	GLuint *indices = new GLuint[NUM_INDICES];
	GLuint *indexPtr = indices;

	vec3 *normals = new vec3[NUM_VERTICES];
	short *shareCount = new short[NUM_VERTICES];
	vec3 v1, v2, v3;
	vec3 va, vb;
	vec3 normal;

	float average;
	int i, j, x, z;

	this -> width = width;
	this -> length = length;
	this -> squareSize = squareSize;
	terrainHeights = new float[width * length];
	memcpy(terrainHeights, heights, sizeof(float) * width * length);
	terrainHeightPtr = terrainHeights;

	for(i = 0; i < width; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			average = 0.0;
			for(z = -1; z <= 1; z ++)
			{
				for(x = -1; x <= 1; x ++)
				{
					if(i + z > 0 && i + z < width &&
					   j + x > 0 && j + x < width)
					{
						average += heights[((i * width) + j) + ((z * width) + x)];
					}
				}
			}
			*terrainHeightPtr++ = average / 8.0;		// instead of 9, let's do 8 for fun
		}
	}

	// posicoes de vertices
	terrainHeightPtr = terrainHeights;
	for(i = 0; i < length; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			*vertexPtr++ = vec3(j * squareSize, (float)*terrainHeightPtr++, -(i * squareSize));
		}
	}

	// textura
	for(i = 0; i < length; i ++)
	{
		for(j = 0; j < width; j ++)
		{
			*baseTexCoordPtr++ = vec2((float)j / ((float)width / 2048.0), (float)i / ((float)length / 2048.0));
		}
	}

	// calcular indices utilizados na renderizacao do terreno;
	// cada quadrado do terreno e construido a partir de 2 triangulos, cada usando 3 dos vertices calculados acima
	for(i = 0; i < length - 1; i ++)
	{
		for(j = 0; j < width - 1; j ++)
		{
			// top left triangle
			*indexPtr++ = (i * width) + j;					// bottom left corner
			*indexPtr++ = ((i + 1) * width) + j;			// top left corner
			*indexPtr++ = ((i + 1) * width) + j + 1;		// top right corner

			// bottom right triangle
			*indexPtr++ = (i * width) + j;					// bottom left corner
			*indexPtr++ = ((i + 1) * width) + j + 1;		// top right corner
			*indexPtr++ = (i * width) + j + 1;				// bottom right corner
		}
	}

	// calcula as normais do terreno
	memset(shareCount, 0, sizeof(short) * NUM_VERTICES);
	for(i = 0; i < NUM_TRIANGLES; i ++)
	{
		// os tres vertices que criam esta face
		v1 = vertices[indices[i * 3]];
		v2 = vertices[indices[(i * 3) + 1]];
		v3 = vertices[indices[(i * 3) + 2]];

		// calcula a normal da face
        va = v2 - v1;
        vb = v3 - v1;
        normal = normalize(cross(va, vb));

        // atribui a normal aos tres vertices que criam esta face
        for(j = 0; j < 3; j ++)
        {
			normals[indices[(i * 3) + j]] += normal;
			shareCount[indices[(i * 3) + j]]++;
        }
	}

	// como as normais esao acumuladas, dividimos
	for(i = 0; i < NUM_VERTICES; i ++)
	{
		normals[i] = normalize(normals[i] / (float)shareCount[i]);
	}

	// cria o vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbos);

	// buffer para o vertice
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * NUM_VERTICES, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// buffer para coordenadas de texturas
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * NUM_VERTICES, baseTexCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// buffer para normais
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * NUM_VERTICES, normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// buffer para indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * NUM_INDICES, indices, GL_STATIC_DRAW);

	// libera memoria auxiliar utilizada no procedimento
	delete[] shareCount;
	delete[] normals;
	delete[] indices;
	delete[] baseTexCoords;
	delete[] vertices;
}




void Terrain::loadShader(){
    //cada regiao do mapa terá uma variavel uniforme correspondente(veja terrain_vertex_shader.glsl e terrain_frag_shader.glsl)
	const float REGION_1_MIN = -200;
	const float REGION_1_MAX = 50;
	const float REGION_2_MIN = REGION_1_MAX;
	const float REGION_2_MAX = 450;
	const float REGION_3_MIN = REGION_2_MAX;
	const float REGION_3_MAX = 650;

	shader = new Shader("../../shaders/terrain_vertex_shader.glsl", "../../shaders/terrain_fragment_shader.glsl");
	if(shader!=NULL){
        shader -> bindAttrib("a_Vertex", 0);
        shader -> bindAttrib("a_BaseTexCoord", 1);
        shader -> bindAttrib("a_Normal", 2);
        shader -> link();
        shader -> bind();

        shader -> uniform1i("u_Region1Tex", 0);
        shader -> uniform1i("u_Region2Tex", 1);
        shader -> uniform1i("u_Region3Tex", 2);
        shader -> uniform1i("u_ShadowTex", 3);
        shader -> uniform1f("u_Region1Max", REGION_1_MAX);
        shader -> uniform1f("u_Region1Range", REGION_1_MAX - REGION_1_MIN);
        shader -> uniform1f("u_Region2Max", REGION_2_MAX);
        shader -> uniform1f("u_Region2Range", REGION_2_MAX - REGION_2_MIN);
        shader -> uniform1f("u_Region3Max", REGION_3_MAX);
        shader -> uniform1f("u_Region3Range", REGION_3_MAX - REGION_3_MIN);

        shader -> uniformVec3("u_Sun", World::SUN_DIRECTION);

        shader -> unbind();

	}else{
        std::cout << "In Terrain::loadShader(): Could not load shader file" << std::endl;
        exit(1);
	}
}

//TODO
void Terrain::render(glm::mat4 &projection, glm::mat4 &model, glm::mat4 &view){

	//mat3 normal = inverseTranspose(mat3(model));

	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(model));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));
	//shader -> uniformMatrix3fv("u_Normal", 1, value_ptr(normal));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, region1Texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, region2Texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, region3Texture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);

	glDisable(GL_BLEND);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (width - 1) * (length - 1) * 6, GL_UNSIGNED_INT, NULL);
}

void Terrain::loadTextures(){
   	region1Texture = loadPNG("../../png/grass.png");
	region2Texture = loadPNG("../../png/rock.png");
	region3Texture = loadPNG("../../png/snow.png");

	if(!region1Texture || !region2Texture || !region3Texture){
        std::cout << "In Terrain::loadTextures() - Could not load the textures" << std::endl;
        std::exit(EXIT_FAILURE);
	}
}
float Terrain::getHeight(glm::vec3 pos){
	int p0, p1, p2, p3;
	float fracX, fracZ;
	float interp0, interp1;
	float result = 0.0;

	// scale the passed-in position into terrain coordinates
	float scaledX = pos.x / squareSize;
	float scaledZ = -pos.z / squareSize;

	// figure out which tile we're standing on
	int tileX = (int)scaledX;		// truncation
	int tileZ = (int)scaledZ;

	// make sure the computed tile positions are actually on the terrain
	if(tileX >= 0 && tileX < width && tileZ >= 0 && tileZ < width)
	{
		// now compute the indices the corners making up the tile we're standing on
		p0 = (tileZ * width + tileX);
		p1 = ((tileZ * width + tileX) + 1);
		p2 = ((tileZ + 1) * width + tileX);
		p3 = ((tileZ + 1) * width + tileX + 1);

		// figure out how much in each direction we've advanced across this tile
		fracX = scaledX - (float)tileX;
		fracZ = scaledZ - (float)tileZ;

		// now interpolate linearly between the height values
		interp0 = terrainHeights[p0] + (terrainHeights[p1] - terrainHeights[p0]) * fracX;
		interp1 = terrainHeights[p2] + (terrainHeights[p3] - terrainHeights[p2]) * fracX;
		result = interp0 + (interp1 - interp0) * fracZ;

	}
	return result;
}



