// createVBO routine and my first experience with VBOs from:
// http://www.songho.ca/opengl/gl_vbo.html

#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#include <GL/gl.h>

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

const char* oclErrorString(cl_int error);

//create a VBO
//target is usually GL_ARRAY_BUFFER 
//usage is usually GL_DYNAMIC_DRAW
GLuint createVBO(const void* data, int dataSize, GLenum target, GLenum usage);


//quick random function to distribute our initial points
float randomFloat(float mn, float mx)
{
    float r = random() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
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

#endif
