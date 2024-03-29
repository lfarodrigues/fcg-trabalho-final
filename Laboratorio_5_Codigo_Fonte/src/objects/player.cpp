#include "player.h"
#include "../world/world.h"
#include "../util/shader.h"
#include "../util/loadmesh.h"
#include "../util/loadtexture.h"
#include "tiny_obj_loader.h"

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
const float Player::GUN_RECOIL_ANIM_TIME = 0.25;		// length of the gun recoil animation in seconds
const float Player::DEATH_IMPACT_ANIM_TIME = 0.5;		// length of the player impact animation in seconds
const int Player::MAX_ROUNDS_PER_CLIP = 12;				// how many bullets can we fire before reloading?

struct ObjModel;

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

    gunWalkBob = 0.0;
    gunWalkBobAmount = 0.0;

    triggerPressed = false;
    gunRecoilFinished = false;

    gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
    gunRecoilAmount = 0.0;

    deathImpactTimer = 0.0;
    deathImpactAmount = 0.0;

    numShotsInClip = MAX_ROUNDS_PER_CLIP;

    gunReloadState = STATE_LOADED;
	gunReloadTimer = 0.0;
	gunReloadOffsetAmount = 0.0;

    alive = true;
    useLookAt = false;
    dtAcum = 0.0;
    //carrega os assets
    loadGunAndSetupVBOs();
    loadTextures();
    loadShader();

    computeWalkingVectors();
    glfwGetCursorPos(window, &oldMouseX, &oldMouseY);
}

Player::~Player(){
}

void Player::loadGunAndSetupVBOs(){
    struct ObjModel model((const char*)"../../mesh/gun.obj");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model.shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model.shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model.shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model.shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model.attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model.attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model.attrib.vertices[3*idx.vertex_index + 2];

                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                if ( idx.normal_index != -1 )
                {
                    const float nx = model.attrib.normals[3*idx.normal_index + 0];
                    const float ny = model.attrib.normals[3*idx.normal_index + 1];
                    const float nz = model.attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model.attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model.attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }
    }

    //glGenBuffers(4, vbos);
    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1;
        number_of_dimensions = 4;
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());

    glBindVertexArray(0);

    numGunVertices = indices.size();
}

void Player::loadTextures(){
    gunDiffuseMap = loadPNG("../../png/gun-diffuse-map.png");
    gunNormalMap = loadPNG("../../png/gun-normal-map.png");
    gunSpecularMap = loadPNG("../../png/gun-specular-map.png");
    gunEmissionMap = loadPNG("../../png/gun-emission-map.png");
}

void Player::loadShader(){
    shader = new Shader("../../shaders/solid-vert.glsl","../../shaders/solid-frag.glsl");
    shader->bindAttrib("a_Vertex", 0);
    shader->bindAttrib("a_Normal", 1);
    shader->bindAttrib("a_TexCoord", 2);
    shader->link();
    shader->bind();
    shader->uniform1i("u_DiffuseMap", 0);
    shader->uniform1i("u_NormalMap", 1);
    shader->uniform1i("u_SpecularMap", 2);
    shader->uniform1i("u_EmissionMap", 3);
    shader->uniformVec3("u_Sun", World::SUN_DIRECTION);
    shader->uniformVec3("u_MaterialDiffuse", glm::vec3(1.0,0.95,0.85));
    shader->uniformVec3("u_MaterialSpecular", glm::vec3(1.0,0.95,0.85));
    shader->uniform1f("u_SpecularIntensity", 8.0);
    shader->uniform1f("u_SpecularHardness", 2.0);
    shader->uniform1f("u_NormalMapStrength", 2.5);
    shader->unbind();
}

void Player::update(float dt){
    if(isAlive()){
        controlMouseInput(dt);
        computeWalkingVectors();
        controlMoving(dt);
        controlGunBobbing(dt);
		controlGunRecoil(dt);
		controlGunReloading(dt);
    }
    controlLooking(dt);
}

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
    const float MOVE_SPEED = 6.7;					// velocidade padrao 6.7 m/s

    const float GRAVITY_STRENGTH = 9.81;            // forca da gravidade
    const float JUMP_ACCEL_TIME = 0.10;				// aceleracao do pulo
	const float JUMP_STRENGTH = 60.0;				// aceleracao maxima do pulo
    const float GUN_INACCURACY = 0.0025;

    glm::vec3 targetVelocity;					    // quao rapido queremos se mover
    glm::vec3 bulletDir;                            // direcao em que disparamos balas
	isMoving = false;

    dtAcum = dtAcum + dt;

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
	if(glfwGetKey(window, 'L') == GLFW_PRESS){
        if (dtAcum >= 1.0){
            if(useLookAt)
                useLookAt = false;
            else
                useLookAt = true;
        }
	}
	// botao esquerdo dispara a arma
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && gunReloadState == STATE_LOADED)
	{
	    // so podemos atirar de novo se ja liberamos o gatilho e a animacao de recuo terminou
		if(!triggerPressed && gunRecoilFinished)
		{
			if(numShotsInClip > 0)
			{
				gunRecoilTimer = 0.0;
				triggerPressed = true;
				numShotsInClip --;

				// compute the bullet direction and fire
				bulletDir = normalize(cameraForward + linearRand(glm::vec3(-GUN_INACCURACY), glm::vec3(GUN_INACCURACY)));
				world->fireBullet(pos, bulletDir);
				//world->addParticle(muzzleFlash, gunPos + pos + (cameraForward * 0.15f) + (cameraUp * 0.065f));
			}
			else
			{
				gunReloadState = STATE_START_MOVING_DOWN;
			}
		}
	}
	else
	{
		triggerPressed = false;
	}

	// V ou R recarrega  a arma, mas isso tambem acontece automaticamente
	if(((glfwGetKey(window, 'V') == GLFW_PRESS || glfwGetKey(window, 'R') == GLFW_PRESS) && gunReloadState == STATE_LOADED && numShotsInClip < MAX_ROUNDS_PER_CLIP) ||
	   (numShotsInClip == 0 && gunRecoilFinished == true && gunReloadState == STATE_LOADED))
	{
		gunReloadState = STATE_START_MOVING_DOWN;
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

void Player::controlGunBobbing(float dt){
    const float GUN_BOB_SPEED = 9.0 * dt;

    if(isMoving){
        gunWalkBob += GUN_BOB_SPEED;
        gunWalkBobAmount += sin(gunWalkBob);
    }
}

void Player::controlGunRecoil(float dt)
{
	const float ALLOW_TRIGGER_PRESS_TIME = GUN_RECOIL_ANIM_TIME * 0.75;

	float recoilFactor;

	gunRecoilTimer += dt;
	gunRecoilFinished = false;

	if(gunRecoilTimer >= GUN_RECOIL_ANIM_TIME)
	{
		gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
	}

	if(gunRecoilTimer >= ALLOW_TRIGGER_PRESS_TIME)
	{
		gunRecoilTimer = GUN_RECOIL_ANIM_TIME;
		gunRecoilFinished = true;
	}

	// we use an inverted, long-tailed quadratic equation to model the recoil animation of the gun
	recoilFactor = gunRecoilTimer / GUN_RECOIL_ANIM_TIME;
	gunRecoilAmount = -((recoilFactor * recoilFactor) - recoilFactor) * pow(abs(recoilFactor - 1.0), 3);
}

void Player::controlGunReloading(float dt)
{
	const float GUN_RELOAD_MOVE_DOWN_TIME = 0.4;				// how long it takes for the gun to lower for reloading
	const float GUN_RELOAD_RELOADING_TIME = 0.4;				// how long it takes for the gun to be reloaded when out of sight
	const float GUN_RELOAD_MOVE_UP_TIME = 0.3;					// how long it takes for the gun to raise into view after reloading

	if(gunReloadState == STATE_START_MOVING_DOWN)
	{
		// the gun begins tilting downwards for a reload after ejecting a clip; play the reload sound
		gunReloadState = STATE_MOVING_DOWN;
		gunReloadTimer = GUN_RELOAD_MOVE_DOWN_TIME;

	}
	else if(gunReloadState == STATE_MOVING_DOWN)
	{
		// the gun is down, so start the reload timer to simulate the time it takes to load a clip
		gunReloadOffsetAmount = 1.0 - (gunReloadTimer / GUN_RELOAD_MOVE_DOWN_TIME);
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_RELOADING;
			gunReloadTimer = GUN_RELOAD_RELOADING_TIME;
			gunReloadOffsetAmount = 1.0;
		}
	}
	else if(gunReloadState == STATE_RELOADING)
	{
		// we wait for the clip to get loaded
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_MOVING_UP;
			gunReloadTimer = GUN_RELOAD_MOVE_UP_TIME;
			numShotsInClip = MAX_ROUNDS_PER_CLIP;
		}
	}
	else if(gunReloadState == STATE_MOVING_UP)
	{
		// clip is loaded, bring the gun back up
		gunReloadOffsetAmount = gunReloadTimer / GUN_RELOAD_MOVE_UP_TIME;
		gunReloadOffsetAmount *= gunReloadOffsetAmount;
		gunReloadTimer -= dt;
		if(gunReloadTimer <= 0.0)
		{
			gunReloadState = STATE_LOADED;
			gunReloadOffsetAmount = 0.0;
		}
	}
}

void Player::computeGunPosition(){
    //posicao da arma � extremidade baixo direita do jogador
    const glm::vec3 GUN_BASE_OFFSET(0.2, -0.125, 0.25);
    //controla a forca do efeito 'bobbing'
    const float GUN_BOB_AMOUNT = 0.006;
    //controla a forca do recuo
    const float GUN_RECOIL_MOVE_STRENGTH = 0.6;
    //controla o offset da anima��o de reload
    const float GUN_RELOAD_MOVE_AMOUNT = -0.25;

    //computa a posi��o base da arma e adiciona o efeito de bobbing, recuo e reload
	gunPos = (cameraForward * GUN_BASE_OFFSET.z) + (cameraSide * GUN_BASE_OFFSET.x) + (cameraUp * GUN_BASE_OFFSET.y);
	gunPos += (cameraSide * gunWalkBobAmount * GUN_BOB_AMOUNT) - (cameraUp * abs(gunWalkBobAmount) * GUN_BOB_AMOUNT);
	gunPos += cameraUp * gunRecoilAmount * GUN_RECOIL_MOVE_STRENGTH;
	gunPos += cameraUp * gunReloadOffsetAmount * GUN_RELOAD_MOVE_AMOUNT;
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

bool Player::getUseLookAt(){
    return useLookAt;
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
