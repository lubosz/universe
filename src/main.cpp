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

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <math.h>

#include <iostream>

#include <string>
#include <fstream>
#include <streambuf>


//OpenGL stuff
//#define GL_GLEXT_PROTOTYPES
//#include <GL/glcorearb.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Our OpenCL Particle Systemclass
#include "Simulator.h"
#include "Renderer.h"

#define NUM_PARTICLES 200000
Simulator* simulator;

//GL related variables
int window_width = 1280;
int window_height = 720;
float translate_z = -1.f;
// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;

GLuint shader_programm;
GLuint vao = 0;

glm::mat4 model;
glm::mat4 projection;

std::vector<std::string> shaders;

//quick random function to distribute our initial points
float randomFloat(float mn, float mx)
{
    float r = random() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
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
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    } else if (mouse_buttons & 4) {
        translate_z += dy * 0.1;
    }

    mouse_old_x = x;
    mouse_old_y = y;

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translate_z));
    model = glm::rotate(model, rotate_x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
}

std::string readFile(const char* fileName) {
    std::ifstream stream(fileName);
    std::string source((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());

    if (source.size() == 0) {
        std::cout << "ERROR: Could not load " << fileName << ".\n";
        return "";
    }

    return source;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (shader_programm);
    glBindVertexArray (vao);

    //this updates the particle system by calling the kernel
    simulator->runKernel();

    //render the particles from VBOs
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(5.);
    
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer(GL_ARRAY_BUFFER, simulator->p_vbo);
    glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray (1);
    glBindBuffer(GL_ARRAY_BUFFER, simulator->c_vbo);
    glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glm::mat4 mvp = projection * model;

    GLint UniformMVP = glGetUniformLocation(shader_programm, "mvp");

    glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &mvp[0][0]);

    glDrawArrays(GL_POINTS, 0, simulator->num);

}

void initShaders() {
    shaders.push_back(readFile("gpu/MVP.vert"));
    shaders.push_back(readFile("gpu/color.frag"));

    const char* vertex[] = {shaders[0].c_str()};
    const char* fragment[] = {shaders[1].c_str()};

    GLuint vs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vs, 1, vertex, NULL);
    glCompileShader (vs);
    GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fs, 1, fragment, NULL);
    glCompileShader (fs);

    shader_programm = glCreateProgram ();
    glAttachShader (shader_programm, fs);
    glAttachShader (shader_programm, vs);
    glLinkProgram (shader_programm);
}

void printContextInfo() {
    // get version info
    const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString (GL_VERSION); // version as a string
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
}

void initGL()
{
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glError;

    model = glm::mat4(1.0f);

    printContextInfo();
    initShaders();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);

    // viewport
    glViewport(0, 0, window_width, window_height);

    float aspect = static_cast<GLfloat>(window_width)
            / static_cast<GLfloat>(window_height);

    projection = glm::perspective(90.0f, aspect, 0.1f, 1000.f);

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translate_z));
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
    glfwSetCursorPosCallback(window, cursorCallback);

    return window;
}

void initParticles(Simulator * simulator) {
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
        float x = rad*sin(2*3.14 * i/num);
        float z = 0.0f;// -.1 + .2f * i/num;
        float y = rad*cos(2*3.14 * i/num);
        pos[i] = Vec4(x, y, z, 1.0f);

        //give some initial velocity
        //float xr = rand_float(-.1, .1);
        //float yr = rand_float(1.f, 3.f);
        //the life is the lifetime of the particle: 1 = alive 0 = dead
        //as you will see in part2.cl we reset the particle when it dies
        float life_r = randomFloat(0.f, 1.f);
        vel[i] = Vec4(0.0, 0.0, 3.0f, life_r);

        //just make them red and full alpha
        color[i] = Vec4(1.0f, 0.0f,0.0f, 1.0f);
    }

    //our load data function sends our initial values to the GPU
    simulator->loadData(pos, vel, color);
}

int main(int argc, char** argv)
{
    GLFWwindow* window = initGLFW();
    //Setup OpenGL related things
    initGL();

    //initialize our CL object, this sets up the context
    simulator = new Simulator();

    std::string kernel_source = readFile("gpu/vortex.cl");

    simulator->loadProgram(kernel_source);

    initParticles(simulator);
    //initialize the kernel
    simulator->initKernel();

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        float aspect = static_cast<GLfloat>(window_width)
                / static_cast<GLfloat>(window_height);

        projection = glm::perspective(90.0f, aspect, 0.1f, 1000.f);

        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
