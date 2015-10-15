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
void initParticles();

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
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        fullscreen = !fullscreen;
        initWindow();
        renderer->bindState(currentWindowWidth,
                            currentWindowHeight);
        renderer->createVertexArray(simulator->positionVBO,
                       simulator->colorVBO,
                       simulator->massVBO);
    }
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        initParticles();
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
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> vel;
    std::vector<glm::vec4> color;
    std::vector<GLfloat> mass;

    float gravity = 0.000000000066742;
    float centralMass = 1000;
    float radius = 20;
    float sateliteMass = 100;

    //create central mass particle
    pos.push_back(glm::vec4(0, 0, 0, 1));
    vel.push_back(glm::vec4(0, 0, 0, 1));
    color.push_back(glm::vec4(1, 0, 0, 1));
    mass.push_back(centralMass);

    //calculate inertial direction/tangent on circle
    glm::vec4 tangent =
            glm::vec4(
                cos(2 * M_PI * 0.1),
                -sin(2 * M_PI * 0.1), 0, 1);
    //normalize tangent
    glm::vec4 normalizedTanent = glm::normalize(tangent);

    //calculate inertial acceleration of satelite to match a stable orbit
    float acceleration = sqrt(gravity * centralMass / radius);

    //create satelite particle
    pos.push_back(glm::vec4(0, radius, 0, 1));
    vel.push_back(glm::vec4(normalizedTanent.x * acceleration,
                       normalizedTanent.y * acceleration,
                       normalizedTanent.z * acceleration,
                       1.0));
    color.push_back(glm::vec4(0, 1, 0, 1));
    mass.push_back(sateliteMass);

    simulator->loadData(pos, vel, color, mass);
}

void initParticles() {
    // initialize our particle system with positions, velocities and color
    const int num = NUM_PARTICLES;

    std::vector<glm::vec4> pos(num);
    std::vector<glm::vec4> vel(num);
    std::vector<glm::vec4> color(num);
    std::vector<GLfloat> mass(num);

    std::random_device rd;
    std::mt19937 e2(rd());

    // std::normal_distribution<> velocityDist(0, .00001);
    float meanRadius = 20;
    std::normal_distribution<> radiusDistribution(meanRadius, meanRadius/2.0);
    std::normal_distribution<> positionDistribution(0.5,0.5);
    std::normal_distribution<> heightDistribution(0, .5);
    std::uniform_real_distribution<> massDistribution(1, 50);

    // std::normal_distribution<> radiusDistribution(0.1, 30);
    // std::uniform_real_distribution<> velocityDist(-.1, .1);
    // std::normal_distribution<> massDist(10, 9.0);

    for (int i = 0; i < num; i++) {
        // distribute the particles in a circle
        float radius = radiusDistribution(e2);
        float arrayPositionRatio = positionDistribution(e2);

        pos[i] = glm::vec4(radius * sin(2 * M_PI * arrayPositionRatio),
                      radius * cos(2 * M_PI * arrayPositionRatio),
                      heightDistribution(e2), 1.0f);

        // distribute masses
        mass[i] = massDistribution(e2);


        // give initial velocity in circle tangent direction
        glm::vec4 tangent = glm::vec4(
                    cos(2 * M_PI * arrayPositionRatio),
                    -sin(2 * M_PI * arrayPositionRatio), 0, 1);
        glm::vec4 normalizedTanent = glm::normalize(tangent);

        float acceleration = 0.001 * (radius/meanRadius*0.75);
        vel[i] = normalizedTanent * acceleration;

        // set color. bigger radius blue, closer to center red
        color[i] = glm::mix(glm::vec4(.9,.1,.1, 1),
                            glm::vec4(.1,.1,.9, 1),
                            radius / 30.0);
    }

    pos[1] = glm::vec4(0, 0, 0, 1);
    vel[1] = glm::vec4(0, 0, 0, 1);
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
    //initSolarSystem();
    simulator->initKernel();

    int printCounter = 0;

    std::chrono::time_point<std::chrono::system_clock> start, physicsStep, graphicsStep;

    renderer->createVertexArray(simulator->positionVBO,
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
