#include "terrain.h"
#include "../util/shader.h"

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <iostream>

Terrain::Terrain(World *world,int width, int length, float squareSize, float *heights){
    this->world = world;

    setupVBOs(width, length, squareSize, heights);
    loadShader();
    //loadTextures();
}



void Terrain::setupVBOs(int width, int length, float squareSize, float *heights){
    const int NUM_VERTICES = width * length;
    const int NUM_INDICES = (width - 1) * (length - 1) * 6;
    const int NUM_TRIANGLES = NUM_INDICES / 3;

    //aloca memoria para vertices
    glm::vec3 *vertices = new glm::vec3[NUM_VERTICES];
    glm::vec3 *vertexPtr = vertices;
    GLfloat *terrainHeightPtr = heights;

    //tetxturas
    //glm::vec2 *baseTexCoords = new glm::vec2[NUM_INDICES];
    //glm::vec2 *baseTexCoordsPtr = baseTexCoords;

    //aloca memoria para indices
    GLuint *indices = new GLuint[NUM_INDICES];
    GLuint *indicesPtr = indices;

    //aloca memoria para normais
    glm::vec3 *normals = new glm::vec3[NUM_VERTICES];
    short *shareCount = new short[NUM_INDICES];

    glm::vec3 v1, v2, v3;
	glm::vec3 va, vb;
	glm::vec3 normal;

	float average;
	int i, j, x, z;

	this->width = width;
	this->length = length;
	this->squareSize = squareSize;
	terrainHeights = new float[width*length];
	memcpy(terrainHeights, heights, sizeof(float)*width*length);
	terrainHeightPtr = terrainHeights;

	//smooth
	for(i = 0; i < width; i++){
        for(j = 0; j < width; j++){
            average = 0.0;
            if(i + z > 0 && i + z < width &&
               j + x > 0 && j + x < width)
            {
                average += heights[((i * width) + j) + ((z * width) + x)];
            }
        }
        *terrainHeightPtr++ = average/8.0;
	}

	//atualiza posicoes dos vertices
    terrainHeightPtr = terrainHeights;
    for(i = 0; i < length; i++){
        for(j = 0; j < width; j++){
            //                          x                   y                          z
            *vertexPtr++ = glm::vec3(j*squareSize, (float)*terrainHeightPtr++, -(i*squareSize));
        }
    }

    //textura

    //computa os indices para renderizacao do terreno
    //cada quadrado do terreno é feito de 2 triangulos, cada usando 3 vertices
    for(i = 0; i < length - 1; i++){
        for(j = 0; j < width - 1; j++){
            //triangulo top left
            *indicesPtr++ = (i*width) + j;         //bottom left
            *indicesPtr++ = ((i+1)*width) + j;     //top left
            *indicesPtr++ = ((i+1)*width) + j + 1; //top right

            //triangulo bottom right
            *indicesPtr++ = (i*width) + j;         //bottom left
            *indicesPtr++ = ((i+1)*width) + j + 1; //top right
            *indicesPtr++ = (i*width) + j + 1;    //bottom right
        }
    }

    //computa as normais do terreno agora que temos os indices
    memset(shareCount, 0, sizeof(short) * NUM_VERTICES);
    for(i = 0; i < NUM_TRIANGLES; i++){
        //obtem os tres vertices da face do triangulo
        v1 = vertices[indices[i*3]];
        v2 = vertices[indices[(i*3) + 1]];
        v3 = vertices[indices[(i*3) + 2]];

        //computa a normal da face
        va = v2 - v1;
        vb = v3 - v1;
        normal = glm::normalize(glm::cross(va,vb));

        //atribui a normal para os tres vertices que compoe o triangulo
        for(j = 0; j < 3; j++){
            normals[indices[(i*3)+j]] += normal;
            shareCount[indices[(i*3)+j]]++;
        }

        //como as normais para cada vertice estao acumuladas acima, precisamos normaliza-las
        for(i = 0; i < NUM_VERTICES; i++){
            normals[i] = normalize(normals[i]/(float)shareCount[i]);
        }

        //criacao de objetos opengl

        //ATENCAO: alteracoes nessa parte do codigo quando comecarmos a trabalhar com textura:
        //      - calcular as coordenadas de textura
        //      - adicionar mais um buffer para enviarmos dados de textura à GPU

        //vertex array
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(3, vbos); // buffers para os atributos de vertices

        //coordenadas
        glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_VERTICES, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(glm::vec3) * NUM_VERTICES, vertices);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        //textures
        /*glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_VERTICES, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(glm::vec3) * NUM_VERTICES, vertices);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);*/

        //normais
        glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_VERTICES, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * NUM_VERTICES, normals);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        //indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

        //desliga o buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //libera a memoria utilizada
        delete[] shareCount;
        delete[] normals;
        delete[] indices;
        delete[] vertices;
    }
}

Terrain::~Terrain(){
    delete[] terrainHeights;
    glDeleteBuffers(3, vbos);
    glDeleteVertexArrays(1, &vao);
    delete shader;
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
	shader->bindAttrib("a_Vertex", 0);
	shader->bindAttrib("a_Normal", 1);
	//bind tex coord

	//link e bind do shader
	shader->link();
	shader->bind();

	//atribuicao de variaveis uniformes
	//variavis de textura
    shader -> uniform1f("u_Region1Max", REGION_1_MAX);
	shader -> uniform1f("u_Region1Range", REGION_1_MAX - REGION_1_MIN);
	shader -> uniform1f("u_Region2Max", REGION_2_MAX);
	shader -> uniform1f("u_Region2Range", REGION_2_MAX - REGION_2_MIN);
	shader -> uniform1f("u_Region3Max", REGION_3_MAX);
	shader -> uniform1f("u_Region3Range", REGION_3_MAX - REGION_3_MIN);

	//direcao do sol

	shader->unbind();
}

//TODO
void Terrain::loadTextures(){

}

void Terrain::render(glm::mat4 &projection, glm::mat4 &model, glm::mat4 &view){

	shader -> bind();
	shader -> uniformMatrix4fv("u_Projection", 1, value_ptr(projection));
	shader -> uniformMatrix4fv("u_Model", 1, value_ptr(model));
	shader -> uniformMatrix4fv("u_View", 1, value_ptr(view));

	//textures

	glDisable(GL_BLEND);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (width - 1) * (length - 1) * 6, GL_UNSIGNED_INT, NULL);
}

//TODO
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



