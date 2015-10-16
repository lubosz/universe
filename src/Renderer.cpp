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


#include "Renderer.h"

#include "util.h"
#include "options.h"
#include <math.h>
#include <gli/gli.hpp>

Renderer::Renderer(int width, int height) {
    scrollPosition = initialScrollPosition;
    theta = initialTheta;
    phi = initialPhi;

    if (gl3wInit()) {
        fprintf(stderr, "failed to initialize gl3w\n");
        return;
    }
    if (!gl3wIsSupported(4, 5)) {
        fprintf(stderr, "OpenGL 4.5 not supported\n");
        return;
    }

    printContextInfo();
    initShaders();
    tex = initTexture("media/cloud-big.dds");
    int pointSize[2];
    glGetIntegerv(GL_POINT_SIZE_RANGE, pointSize);
    printf("Point Size Min %d Max %d\n", pointSize[0], pointSize[1]);

    bindState(width, height);

    glError;
}

void Renderer::bindState(int width, int height) {
    glUseProgram(shader_programm);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLint texLoc = glGetUniformLocation(shader_programm, "cloud");
    glUniform1i(texLoc, tex-1);
    updateView();
    updateProjection(width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(vao);
}

Renderer::~Renderer() {
    GLuint textures[1];
    textures[0] = textureId;
    glDeleteTextures(1, textures);
}

void Renderer::createVertexArray(GLuint positionVBO,
                                 GLuint colorVBO,
                                 GLuint massVBO) {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, massVBO);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
}

void Renderer::draw(int particleCount) {
  // render the particles from VBOs
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawArrays(GL_POINTS, 0, particleCount);
}

void Renderer::printContextInfo() {
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    printf("GL_SHADING_LANGUAGE_VERSION: %s\n",
           glGetString(GL_SHADING_LANGUAGE_VERSION));
}

GLuint Renderer::initTexture(char const* Filename) {
    gli::texture Texture = gli::load(Filename);
    if (Texture.empty())
        return 0;

    gli::gl GL;
    gli::gl::format const Format = GL.translate(Texture.format());
    GLenum Target = GL.translate(Texture.target());

    glGenTextures(1, &textureId);
    glBindTexture(Target, textureId);
    glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(Target,
        GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));

    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::tvec3<GLsizei> const Dimensions(Texture.dimensions());
    GLsizei const FaceTotal =
            static_cast<GLsizei>(Texture.layers() * Texture.faces());

    glTexStorage2D(
                Target, static_cast<GLint>(Texture.levels()), Format.Internal,
                Dimensions.x,
                Texture.target() == gli::TARGET_2D ? Dimensions.y : FaceTotal);

    for (std::size_t Layer = 0;
         Layer < Texture.layers(); ++Layer)
        for (std::size_t Face = 0;
             Face < Texture.faces(); ++Face)
            for (std::size_t Level = 0;
                 Level < Texture.levels(); ++Level) {
                glm::tvec3<GLsizei> Dimensions(Texture.dimensions(Level));

                /*
                printf("layer %ld, face %ld, level %ld, iscompressed %d\n",
                       Layer, Face, Level,
                       gli::is_compressed(Texture.format()));
                */
                if (gli::is_compressed(Texture.format()))
                    glCompressedTexSubImage2D(
                                Target, static_cast<GLint>(Level),
                                0, 0,
                                Dimensions.x,
                                Dimensions.y,
                                Format.Internal,
                                static_cast<GLsizei>(Texture.size(Level)),
                                Texture.data(Layer, Face, Level));
                else
                    glTexSubImage2D(
                                Target, static_cast<GLint>(Level),
                                0, 0,
                                Dimensions.x,
                                Dimensions.y,
                                Format.External, Format.Type,
                                Texture.data(Layer, Face, Level));
            }
    return textureId;
}

static void printProgramInfoLog(GLuint program) {
  int infologLen = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen);
  if (infologLen > 1) {
    GLchar * infoLog = new GLchar[infologLen];
    glGetProgramInfoLog(program, infologLen, NULL, infoLog);
    std::cout << "Error compiling program" << infoLog << "\n";
    delete[] infoLog;
  }
}

void Renderer::initShaders() {
    shaders.push_back(readFile("gpu/MVP.vert"));
    shaders.push_back(readFile("gpu/color.frag"));

    const char* vertex[] = {shaders[0].c_str()};
    const char* fragment[] = {shaders[1].c_str()};

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, vertex, NULL);
    glCompileShader(vs);
    printShaderInfoLog(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, fragment, NULL);
    glCompileShader(fs);
    printShaderInfoLog(fs);

    shader_programm = glCreateProgram();
    glAttachShader(shader_programm, fs);
    glAttachShader(shader_programm, vs);
    glLinkProgram(shader_programm);
    printProgramInfoLog(shader_programm);
}

void Renderer::printShaderInfoLog(GLuint shader) {
  int infologLen = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
  if (infologLen > 1) {
    GLchar * infoLog = new GLchar[infologLen];
    glGetShaderInfoLog(shader, infologLen, NULL, infoLog);
    std::cout << "Error compiling shader" << infoLog << "\n";
    delete[] infoLog;
  }
}

void Renderer::rotate(float x, float y) {
    theta += y * rotationSpeed;
    phi += x * rotationSpeed;
    updateView();
}

void Renderer::translate(float z) {
    scrollPosition += z * scrollSpeed;
    updateView();
}

void Renderer::updateView() {
    float radius = exp(-scrollPosition);
    glm::vec3 center(0, 0, 0);
    glm::vec3 up(0, 0, -1);

    glm::vec3 eye(radius * sin(theta) * cos(phi),
                  radius * sin(theta) * sin(phi),
                  radius * cos(theta));

    // printf("theta %f phi %f scrollPosition %f\n",
    //   theta, phi, scrollPosition);

    view = glm::lookAt(eye, center, up);

    glUniformMatrix4fv(
                glGetUniformLocation(shader_programm, "viewMatrix"),
                1, GL_FALSE, &view[0][0]);
}

void Renderer::updateProjection(int width, int height) {
    glViewport(0, 0, width, height);
    float aspect = static_cast<GLfloat>(width)
            / static_cast<GLfloat>(height);
    projection = glm::perspective(fov, aspect, 0.01f, 10000.f);
    glUniformMatrix4fv(
                glGetUniformLocation(shader_programm, "projectionMatrix"),
                1, GL_FALSE, &projection[0][0]);
}

void Renderer::checkGlError(const char* file, int line) {
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
        std::cout
                << "GL ERROR: GL_"
                << error << " "
                << file << " "
                << line << "\n";
        err = glGetError();
    }
}

GLuint Renderer::createVBO(
        const void* data,
        int dataSize,
        GLenum target,
        GLenum usage) {
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(target, id);
    glBufferData(target, dataSize, data, usage);

    // check data size in VBO is same as input array
    // if not return 0 and delete VBO
    int bufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
    if (dataSize != bufferSize) {
        glDeleteBuffers(1, &id);
        id = 0;
        std::cout << "[createVBO()] Data size is mismatch with input array\n";
    }
    glBindBuffer(target, 0);
    return id;
}
