#include "Renderer.h"

#include "util.h"

Renderer::Renderer()
{
    translate_z = -1.f;
    rotate_x = 0.0;
    rotate_y = 0.0;
}

Renderer::~Renderer() {}

void Renderer::draw(GLuint positionVBO, GLuint colorVBO, int particleCount) {
  //render the particles from VBOs
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (shader_programm);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPointSize(5.);

  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);
  glEnableVertexAttribArray (0);
  glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
  glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray (1);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  glm::mat4 mvp = projection * model;

  GLint UniformMVP = glGetUniformLocation(shader_programm, "mvp");

  glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &mvp[0][0]);

  glDrawArrays(GL_POINTS, 0, particleCount);
}

void Renderer::printContextInfo() {
    // get version info
    const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString (GL_VERSION); // version as a string
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
}

void Renderer::initShaders() {
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

void Renderer::initGL(int width, int height)
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
    glViewport(0, 0, width, height);
    updateProjection(width, height);

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translate_z));
}

void Renderer::bindVAO() {
    glBindVertexArray (vao);
}

void Renderer::rotate(float x, float y) {
    rotate_x += y * 0.2;
    rotate_y += x * 0.2;
}

void Renderer::translate(float y) {
    translate_z += y * 0.1;
}

void Renderer::updateView() {
    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, translate_z));
    model = glm::rotate(model, rotate_x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotate_y, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Renderer::updateProjection(int width, int height) {
    float aspect = static_cast<GLfloat>(width)
            / static_cast<GLfloat>(height);
    projection = glm::perspective(90.0f, aspect, 0.1f, 1000.f);
}
