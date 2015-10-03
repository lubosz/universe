/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "Renderer.h"
#include "Simulator.h"
#include "util.h"
#include <math.h>
#include <random>

#define NUM_PARTICLES 9000

Simulator* simulator;
Renderer* renderer;

int window_width = 1920;
int window_height = 1080;

int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;

static void windowFrameBufferCallback(
        GLFWwindow * window, int width, int height) {
    renderer->updateProjection(width, height);
}

static void errorCallback(int error, const char* description) {
    fputs(description, stderr);
}
static void keyCallback(
        GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    renderer->translate(yoffset);
}

void buttonCallback(GLFWwindow* window, int button, int action, int mods) {
    // handle mouse interaction for rotating/zooming the view
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_buttons |= 1 << button;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_buttons = 0;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    mouse_old_x = x;
    mouse_old_y = y;
}

static void cursorCallback(GLFWwindow* window, double x, double y) {
    // hanlde the mouse motion for zooming and rotating the view
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if (mouse_buttons & 1) {
        renderer->rotate(dx, dy);
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

GLFWwindow* initGLFW() {
    GLFWwindow* window;
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
                window_width, window_height,
                "Universe Simulator", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, buttonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetFramebufferSizeCallback(window, windowFrameBufferCallback);

    return window;
}

void initParticles() {
    // initialize our particle system with positions, velocities and color
    int num = NUM_PARTICLES;

    std::vector<Vec4> pos(num);
    std::vector<Vec4> vel(num);
    std::vector<Vec4> color(num);
    std::vector<GLfloat> mass(num);

    std::random_device rd;
    std::mt19937 e2(rd());

    std::normal_distribution<> velocityDist(0, .00001);
    std::normal_distribution<> posDist(0, .1);
    std::normal_distribution<> massDist(10, 9);
    // std::uniform_real_distribution<> velocityDist(-.1, .1);
    // std::normal_distribution<> massDist(10, 9.0);

    // fill our vectors with initial data
    for (int i = 0; i < num; i++) {
        /*
        // distribute the particles in a random circle around z axis
        float rad = randomFloat(.2, .5);
        // float rad = 1.0;
        float x = rad * sin(2 * M_PI * i/num);
        float z = 0.0f;  // -.1 + .2f * i/num;
        float y = rad * cos(2 * M_PI * i/num);
        */

        pos[i] = Vec4(posDist(e2),
                      posDist(e2),
                      posDist(e2), 1.0f);

        mass[i] = fabs(massDist(e2));

        // give some initial velocity
        // float xr = rand_float(-.1, .1);
        // float yr = rand_float(1.f, 3.f);
        // the life is the lifetime of the particle: 1 = alive 0 = dead
        // as you will see in part2.cl we reset the particle when it dies
        // float life_r = randomFloat(0.f, 1.f);
        // vel[i] = Vec4(0.0, 0.0, 3.0f, life_r);

        /*
        vel[i] =
                Vec4(
                    velocityDist(e2),
                    velocityDist(e2),
                    velocityDist(e2),
                    1);
        */

        vel[i] = Vec4(0, 0, 0, 1);
        /*
        glm::vec4 position(pos[i].x, pos[i].y, pos[i].z, 1.0);
        float r = glm::length(position);
        // float r = sqrt(pow(pos[i].x,2) + pow(pos[i].y,2) + pow(pos[i].z,2));


        float theta = acos(pos[i].z/r);
        float phi = atan2(pos[i].y, pos[i].x);

        // then add pi/2 to theta or phi
        theta += M_PI/2.0;

             vel[i] =
                        Vec4(
                            sin(theta) * cos(phi),
                            sin(theta) * sin(phi),
                            cos(theta),
                            1);
        */
        // just make them red and full alpha
        color[i] = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // our load data function sends our initial values to the GPU
    simulator->loadData(pos, vel, color, mass);
}

int main(int argc, char** argv) {
    GLFWwindow* window = initGLFW();
    renderer = new Renderer(window_width, window_height);

    std::string kernel_source = readFile("gpu/vortex.cl");
    simulator = new Simulator();
    simulator->loadProgram(kernel_source);

    initParticles();
    simulator->initKernel();

    while (!glfwWindowShouldClose(window)) {
        simulator->runKernel();
        renderer->draw(simulator->positionVBO,
                       simulator->colorVBO,
                       simulator->massVBO,
                       simulator->particleCount);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
