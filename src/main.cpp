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

#define NUM_PARTICLES 5000

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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        simulator->dt = 100.0;
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
        simulator->dt = 10.0;
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

void initSolarSystem() {
    std::vector<Vec4> pos;
    std::vector<Vec4> vel;
    std::vector<Vec4> color;
    std::vector<GLfloat> mass;

    //sun

    pos.push_back(Vec4(0,0,0,1));
    vel.push_back(Vec4(0,0,0,1));
    color.push_back(Vec4(0,0,0,1));
    mass.push_back(333000.0);

    //2020 / 333000.0;

    // earth
    // distance ×10^8 km
    pos.push_back(Vec4(1.470,0,0,1));
    vel.push_back(Vec4(0,.005,0,1));
    color.push_back(Vec4(0,0,0,1));
    mass.push_back(1.0);


    // jupiter
    // Perihelion
    pos.push_back(Vec4(7.405736,0,0,1));
    vel.push_back(Vec4(0,-.005,0,1));
    color.push_back(Vec4(0,0,0,1));
    mass.push_back(317.8);


    simulator->loadData(pos, vel, color, mass);
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

    //std::normal_distribution<> velocityDist(0, .00001);
    std::normal_distribution<> posDist(5, 2);
    std::normal_distribution<> heightDist(0, .5);
    std::uniform_real_distribution<> massDist(1, 1000);
    // std::uniform_real_distribution<> velocityDist(-.1, .1);
    // std::normal_distribution<> massDist(10, 9.0);

    // fill our vectors with initial data
    for (int i = 0; i < num; i++) {

        // distribute the particles in a random circle around z axis
        float rad = posDist(e2);

        /*
        // float rad = 1.0;
        float x = rad * sin(2 * M_PI * i/num);
        float z = 0.0f;  // -.1 + .2f * i/num;
        float y = rad * cos(2 * M_PI * i/num);
        */

        pos[i] = Vec4(rad * sin(2 * M_PI * float(i)/float(num)),
                      rad * cos(2 * M_PI * float(i)/float(num)),
                      heightDist(e2), 1.0f);

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
        glm::vec4 tangent =
                glm::vec4(
                    cos(2 * M_PI * float(i)/float(num)),
                    -sin(2 * M_PI * float(i)/float(num)),0, 1);
        glm::vec4 normalizedTanent = glm::normalize(tangent);

        float acceleration = 0.0025;

        vel[i] = Vec4(normalizedTanent.x * acceleration,
                      normalizedTanent.y * acceleration,
                      normalizedTanent.z * acceleration,
                      1.0);

        /**/
        // just make them red and full alpha
        color[i] = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
/*
    pos[1] = Vec4(0,0,0,1);
    vel[1] = Vec4(0,0,0,1);
    mass[1] = 2000;
*/
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
    //initSolarSystem();
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
