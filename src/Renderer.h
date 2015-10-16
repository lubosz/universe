/* Universe
 *
 * The MIT License (MIT)
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

    GLuint textureId = 0;

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

    void draw(int particleCount);
    static void checkGlError(const char* file, int line);

    GLuint initTexture(char const* Filename);

    void bindState(int width, int height);

    void createVertexArray(GLuint positionVBO,
                               GLuint colorVBO,
                               GLuint massVBO);

    static GLuint createVBO(
            const void* data,
            int dataSize,
            GLenum target,
            GLenum usage);
};

#endif  // SRC_RENDERER_H_

