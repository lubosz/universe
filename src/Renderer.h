#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <iostream>
#include <vector>

//OpenGL stuff
#include <GL/glew.h>
# define GLCOREARB_PROTOTYPES 1
# define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void checkGlError(const char* file, int line) {
    GLenum err(glGetError());

    while (err != GL_NO_ERROR) {
        std::string error;
        switch (err) {
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                error = "Unknown error";
                break;
        }
        std::cout << "GL ERROR: GL_" << error << " " << file << " " << line << "\n";
        err = glGetError();
    }
}

#define glError checkGlError(__FILE__,__LINE__)


class Renderer {
public:

    GLuint shader_programm;
    GLuint vao = 0;

    glm::mat4 model;
    glm::mat4 projection;

    std::vector<std::string> shaders;

    float translate_z;
    float rotate_x;
    float rotate_y;

    Renderer();
    ~Renderer();

    void printContextInfo();
    void initShaders();
    void initGL(int width, int height);
    void bindVAO();

    void updateView();
    void updateProjection(int width, int height);

    void rotate(float x, float y);
    void translate(float y);

    void draw(GLuint positionVBO, GLuint colorVBO, int particleCount);
};

#endif // RENDERER_H

