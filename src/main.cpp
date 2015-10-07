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
#include "options.h"
#include <math.h>
#include <random>
#include <ctime>
#include <chrono>

Simulator* simulator;
Renderer* renderer = NULL;
GLFWwindow* window = NULL;
bool fullscreen = true;

int currentWindowWidth = window_width;
int currentWindowHeight = window_height;

int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;

void initWindow();

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
        simulator->dt = fastDt;
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
        simulator->dt = slowDt;
    if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
        fullscreen = !fullscreen;
        initWindow();
        renderer->bindState(currentWindowWidth,
                            currentWindowHeight);
        renderer->initBuffers(simulator->positionVBO,
                       simulator->colorVBO,
                       simulator->massVBO);
    }
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

void initWindow() {
    GLFWwindow * newWindow;

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        currentWindowWidth = mode->width;
        currentWindowHeight = mode->height;

        newWindow = glfwCreateWindow(currentWindowWidth,
                                     currentWindowHeight,
                                     title.c_str(),
                                     glfwGetPrimaryMonitor(),
                                     window);
    } else {
        currentWindowWidth = window_width;
        currentWindowHeight = window_height;

        newWindow = glfwCreateWindow(currentWindowWidth,
                                     currentWindowHeight,
                                     title.c_str(),
                                     NULL,
                                     window);
    }

    glfwDestroyWindow(window);

    if (!newWindow) {
        printf("ERROR: Could not create GLFW Window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(newWindow);
    glfwSwapInterval(0);
    glfwSetKeyCallback(newWindow, keyCallback);
    glfwSetMouseButtonCallback(newWindow, buttonCallback);
    glfwSetScrollCallback(newWindow, scrollCallback);
    glfwSetCursorPosCallback(newWindow, cursorCallback);
    glfwSetFramebufferSizeCallback(newWindow, windowFrameBufferCallback);

    window = newWindow;
}

void initSolarSystem() {
    std::vector<Vec4> pos;
    std::vector<Vec4> vel;
    std::vector<Vec4> color;
    std::vector<GLfloat> mass;

    // sun

    pos.push_back(Vec4(0, 0, 0, 1));
    vel.push_back(Vec4(0, 0, 0, 1));
    color.push_back(Vec4(0, 0, 0, 1));
    mass.push_back(333000.0);

    // 2020 / 333000.0;

    // earth
    // distance ×10^8 km
    pos.push_back(Vec4(1.470, 0, 0, 1));
    vel.push_back(Vec4(0, .005, 0, 1));
    color.push_back(Vec4(0, 0, 0, 1));
    mass.push_back(1.0);


    // jupiter
    // Perihelion
    pos.push_back(Vec4(7.405736, 0, 0 , 1));
    vel.push_back(Vec4(0, -.005, 0, 1));
    color.push_back(Vec4(0, 0, 0, 1));
    mass.push_back(317.8);


    simulator->loadData(pos, vel, color, mass);
}

void initParticles() {
    // initialize our particle system with positions, velocities and color
    const int num = NUM_PARTICLES;

    std::vector<Vec4> pos(num);
    std::vector<Vec4> vel(num);
    std::vector<Vec4> color(num);
    std::vector<GLfloat> mass(num);

    std::random_device rd;
    std::mt19937 e2(rd());

    // std::normal_distribution<> velocityDist(0, .00001);
    std::normal_distribution<> radiusDistribution(20, 10);
    std::normal_distribution<> heightDistribution(0, .5);
    std::uniform_real_distribution<> massDistribution(1, 1000);

    // std::normal_distribution<> radiusDistribution(0.1, 30);
    // std::uniform_real_distribution<> velocityDist(-.1, .1);
    // std::normal_distribution<> massDist(10, 9.0);

    // fill our vectors with initial data
    for (int i = 0; i < num; i++) {
        // distribute the particles in a random circle around z axis
        float radius = radiusDistribution(e2);

        /*
        // float rad = 1.0;
        float x = rad * sin(2 * M_PI * i/num);
        float z = 0.0f;  // -.1 + .2f * i/num;
        float y = rad * cos(2 * M_PI * i/num);
        */

        float arrayPositionRatio =
                static_cast<float>(i) / static_cast<float>(num);

        pos[i] = Vec4(radius * sin(2 * M_PI * arrayPositionRatio),
                      radius * cos(2 * M_PI * arrayPositionRatio),
                      heightDistribution(e2), 1.0f);

        mass[i] = fabs(massDistribution(e2));

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
                    cos(2 * M_PI * arrayPositionRatio),
                    -sin(2 * M_PI * arrayPositionRatio), 0, 1);
        glm::vec4 normalizedTanent = glm::normalize(tangent);

        float acceleration = 0.0001 * (radius / 30.0);


        vel[i] = Vec4(normalizedTanent.x * acceleration,
                      normalizedTanent.y * acceleration,
                      normalizedTanent.z * acceleration,
                      1.0);

        glm::vec3 starColor = glm::mix(glm::vec3(.9,.1,.1),
                                       glm::vec3(.1,.1,.9),
                                       radius / 30.0);
        color[i] = Vec4(starColor.x,
                        starColor.y,
                        starColor.z,
                        1.0f);
    }

    pos[1] = Vec4(0, 0, 0, 1);
    vel[1] = Vec4(0, 0, 0, 1);
    mass[1] = bigMass;

    // our load data function sends our initial values to the GPU
    simulator->loadData(pos, vel, color, mass);
}

int main(int argc, char** argv) {
    initWindow();
    renderer = new Renderer(currentWindowWidth,
                            currentWindowHeight);
    std::string kernel_source = readFile("gpu/vortex.cl");
    simulator = new Simulator();
    simulator->loadProgram(kernel_source);

    initParticles();
    // initSolarSystem();
    simulator->initKernel();

    int printCounter = 0;

    std::chrono::time_point<std::chrono::system_clock> start, physicsStep, graphicsStep;

    renderer->initBuffers(simulator->positionVBO,
                   simulator->colorVBO,
                   simulator->massVBO);

    while (!glfwWindowShouldClose(window)) {
        start = std::chrono::system_clock::now();
        simulator->runKernel();
        physicsStep = std::chrono::system_clock::now();

        renderer->draw(simulator->particleCount);
        glfwSwapBuffers(window);

        glFinish();

        graphicsStep = std::chrono::system_clock::now();

        glfwPollEvents();

        printCounter--;
        if (printCounter < 0) {
            std::cout << "simulation: "
                      << std::chrono::duration_cast<std::chrono::microseconds>
                         (physicsStep-start).count() / 1000.0
                      << "ms\n";

            std::cout << "graphics: "
                      << std::chrono::duration_cast<std::chrono::microseconds>
                         (graphicsStep-physicsStep).count() / 1000.0
                      << "ms\n";

            float totalMs = std::chrono::duration_cast<std::chrono::microseconds>
                    (graphicsStep-start).count() / 1000.0;

            std::cout << "total: " << totalMs << "ms (" << 1.0 / (totalMs / 1000.0) << " fps)\n";
            printCounter = 10;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
