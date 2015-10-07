/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#ifndef SRC_RENDERER_H_
#define SRC_RENDERER_H_

#include <string>
#include <iostream>
#include <vector>

# define GLCOREARB_PROTOTYPES 1
# define GL_GLEXT_PROTOTYPES 1
#include "GL/glcorearb.h"
#include "GL/gl3w.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define glError Renderer::checkGlError(__FILE__, __LINE__)

class Renderer {
 public:
    GLuint shader_programm;
    GLuint vao = 0;
    GLuint tex;

    glm::mat4 view;
    glm::mat4 projection;

    std::vector<std::string> shaders;

    float scrollPosition;
    float theta;
    float phi;

    Renderer(int width, int height);
    ~Renderer();

    void printContextInfo();
    void printShaderInfoLog(GLuint shader);
    void initShaders();
    void initGL(int width, int height);
    void bindVAO();

    void updateView();
    void updateProjection(int width, int height);

    void rotate(float x, float y);
    void translate(float z);

    void draw(GLuint positionVBO, GLuint colorVBO,
              GLuint massVBO, int particleCount);
    static void checkGlError(const char* file, int line);

    GLuint initTexture(char const* Filename);

    void bindState(int width, int height);

    static GLuint createVBO(
            const void* data,
            int dataSize,
            GLenum target,
            GLenum usage);
};

#endif  // SRC_RENDERER_H_

