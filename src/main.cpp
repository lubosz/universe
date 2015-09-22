/*
 * Adventures in OpenCL tutorial series
 * Part 2
 *
 * author: Ian Johnson
 * htt://enja.org
 * code based on advisor Gordon Erlebacher's work
 * NVIDIA's examples
 * as well as various blogs and resources on the internet
 *
 * Ported to GL4 and GFLW by Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "Renderer.h"
#include "Simulator.h"
#include "util.h"
#include <math.h>

#define NUM_PARTICLES 200

Simulator* simulator;
Renderer* renderer;

//GL related variables
int window_width = 1280;
int window_height = 720;
// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;


static void windowFrameBufferCallback(GLFWwindow * 	window, int width, int height) {
    renderer->updateProjection(width, height);
}


static void errorCallback(int error, const char* description)
{
    fputs(description, stderr);
}
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    renderer->translate(yoffset);
}

void buttonCallback(GLFWwindow* window, int button, int action, int mods)
{
    //handle mouse interaction for rotating/zooming the view
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_buttons |= 1<<button;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_buttons = 0;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    mouse_old_x = x;
    mouse_old_y = y;
}

static void cursorCallback(GLFWwindow* window, double x, double y)
{
    //hanlde the mouse motion for zooming and rotating the view
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

    window = glfwCreateWindow(window_width, window_height, "Universe Simulator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, buttonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetFramebufferSizeCallback(window, windowFrameBufferCallback);

    return window;
}

void initParticles() {
    //initialize our particle system with positions, velocities and color
    int num = NUM_PARTICLES;
    //int num = 1;
    std::vector<Vec4> pos(num);
    std::vector<Vec4> vel(num);
    std::vector<Vec4> color(num);

    //fill our vectors with initial data
    for(int i = 0; i < num; i++)
    {
        //distribute the particles in a random circle around z axis
        float rad = randomFloat(.2, .5);
        //float rad = 1.0;
        float x = rad * sin(2 * M_PI * i/num);
        float z = 0.0f;// -.1 + .2f * i/num;
        float y = rad * cos(2 * M_PI * i/num);
        pos[i] = Vec4(x, y, z, 1.0f);

        //give some initial velocity
        //float xr = rand_float(-.1, .1);
        //float yr = rand_float(1.f, 3.f);
        //the life is the lifetime of the particle: 1 = alive 0 = dead
        //as you will see in part2.cl we reset the particle when it dies
        //float life_r = randomFloat(0.f, 1.f);
        //vel[i] = Vec4(0.0, 0.0, 3.0f, life_r);
        vel[i] =
                Vec4(
                    randomFloat(-1.f, 1.f),
                    randomFloat(-1.f, 1.f),
                    randomFloat(-1.f, 1.f),
                    1.0);

        //just make them red and full alpha
        color[i] = Vec4(1.0f, 0.0f,0.0f, 1.0f);
    }

    //our load data function sends our initial values to the GPU
    simulator->loadData(pos, vel, color);
}

int main(int argc, char** argv)
{
    GLFWwindow* window = initGLFW();
    renderer = new Renderer(window_width, window_height);

    std::string kernel_source = readFile("gpu/vortex.cl");
    simulator = new Simulator();
    simulator->loadProgram(kernel_source);

    initParticles();
    //initialize the kernel
    simulator->initKernel();

    while (!glfwWindowShouldClose(window))
    {
        //this updates the particle system by calling the kernel
        simulator->runKernel();
        renderer->draw(simulator->positionVBO,
                       simulator->colorVBO,
                       simulator->particleCount);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
