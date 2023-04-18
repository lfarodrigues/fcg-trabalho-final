/*           Trabalho Final
    Fundamentos de Computação Gráfica(INF01047)
    Aluno: Luis Filipe (00314848)
    Semestre: 2022/02
*/

/*Estrutura de pastas
Projeto
    //code
    src
    |--> objects
            |--> drone.cpp
    |--> world
            |--> terrain.cpp
            |--> grass.cpp
            |--> sky.cpp
            |--> world.cpp
    //headers
    src
    |--> objects
            |--> drone.h

    |--> world
            |--> terrain.h
            |--> grass.h
            |--> sky.h
            |--> world.h
    //data
    mesh
    png
    icons
    wav
    shaders
*/
#include "world/world.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

void FrameBufferSizeCallback(GLFWwindow *window, int width, int height);

GLFWwindow *window;
glm::vec2 windowSize;       // alterado por openWindow()

void openWindow();
void prepareOpenGL();

int main() {
    const string WORLD_FILE = "../../png/world.png";

    World *world;

    //constantes para controle de timing dos frames
    const double TARGET_FRAME_INTERVAL = 0.016665;      // tempo entre frames
    const double FRAME_TIME_ALPHA = 0.25;               // para computacao do delta

    double dt;
    double currentTime;
    double oldTime;
    double frameTime = 0.0;
    double renderAccum = 0.0;
    double smoothFrameTime = TARGET_FRAME_INTERVAL;

    srand(time(NULL));

    openWindow();
    prepareOpenGL();

    world = new World(window, windowSize, WORLD_FILE);

    if(world!=NULL){
        currentTime = glfwGetTime();
        oldTime = currentTime;

        while(!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) && !world->isGameDone()){


            oldTime = currentTime;
            currentTime = glfwGetTime();
            dt = currentTime - oldTime;

            renderAccum += dt;
            frameTime += dt;

            if(renderAccum >= TARGET_FRAME_INTERVAL){
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                renderAccum -= TARGET_FRAME_INTERVAL;

                smoothFrameTime = (smoothFrameTime * FRAME_TIME_ALPHA) + (frameTime * (1.0 - FRAME_TIME_ALPHA));
                frameTime = 0.0;


                world->update(smoothFrameTime);
                world->render();

                glfwPollEvents();
                glfwSwapBuffers(window);
            }
        }
    } else {
        std::cout << "In main() - could not instantiate world object" << std::endl;
    }

    delete world;
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void openWindow(){
    const char *TITLE = "Trabalho FCG - Dupla Luis Filipe e Guilherme";

    int windowWidth;
    int windowHeight;

    if(!glfwInit()){
        cerr << "openWindow() could not initialize GLFW" << endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_REFRESH_RATE, 60);
    #ifdef DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif // DEBUG

    //const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    windowWidth = 1200;
    windowHeight = 600;

    window = glfwCreateWindow(windowWidth, windowHeight,
                               TITLE,
                               NULL,
                               NULL);

    if(!window){
        glfwTerminate();
        std::cerr << "Could not create window" << std::endl;
        std::exit(EXIT_FAILURE);
    }


    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    FrameBufferSizeCallback(window, windowWidth, windowHeight);

    windowSize = glm::vec2(windowWidth, windowHeight);
}

void prepareOpenGL(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529, 0.808, 0.922, 0.0);
}

void FrameBufferSizeCallback(GLFWwindow *window, int width, int height){
    glViewport(0,0,width,height);
}
