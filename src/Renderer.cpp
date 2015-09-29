/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#include "Renderer.h"

#include "util.h"

Renderer::Renderer(int width, int height) {
    translate_z = -1.f;
    rotate_x = 0.0;
    rotate_y = 0.0;

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glError;

    printContextInfo();
    initShaders();
    glUseProgram(shader_programm);

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    updateModel();
    updateProjection(width, height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glPointSize(5.);

    glBindVertexArray(vao);
}

Renderer::~Renderer() {}

void Renderer::draw(GLuint positionVBO, GLuint colorVBO, int particleCount) {
  // render the particles from VBOs
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glDrawArrays(GL_POINTS, 0, particleCount);
}

void Renderer::printContextInfo() {
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
}

void Renderer::initShaders() {
    shaders.push_back(readFile("gpu/MVP.vert"));
    shaders.push_back(readFile("gpu/color.frag"));

    const char* vertex[] = {shaders[0].c_str()};
    const char* fragment[] = {shaders[1].c_str()};

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, vertex, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, fragment, NULL);
    glCompileShader(fs);

    shader_programm = glCreateProgram();
    glAttachShader(shader_programm, fs);
    glAttachShader(shader_programm, vs);
    glLinkProgram(shader_programm);
}

void Renderer::rotate(float x, float y) {
    rotate_x += y * 0.2;
    rotate_y += x * 0.2;
    updateModel();
}

void Renderer::translate(float z) {
    translate_z += z * 0.1;
    updateModel();
}

void Renderer::updateModel() {
    model = glm::mat4(1.0f);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translate_z));
    model = glm::rotate(model, rotate_x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
    updateMVP();
}

void Renderer::updateProjection(int width, int height) {
    glViewport(0, 0, width, height);
    float aspect = static_cast<GLfloat>(width)
            / static_cast<GLfloat>(height);
    projection = glm::perspective(90.0f, aspect, 0.1f, 1000.f);
    updateMVP();
}

void Renderer::updateMVP() {
    glm::mat4 mvp = projection * model;
    GLint UniformMVP = glGetUniformLocation(shader_programm, "mvp");
    glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &mvp[0][0]);
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
