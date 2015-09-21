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



#define glError Renderer::checkGlError(__FILE__,__LINE__)


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

    void updateModel();
    void updateProjection(int width, int height);
    void updateMVP();

    void rotate(float x, float y);
    void translate(float z);

    void draw(GLuint positionVBO, GLuint colorVBO, int particleCount);
    static void checkGlError(const char* file, int line);

    static GLuint createVBO(const void* data, int dataSize, GLenum target, GLenum usage);

};

#endif // RENDERER_H

