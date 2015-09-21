// createVBO routine and my first experience with VBOs from:
// http://www.songho.ca/opengl/gl_vbo.html

#ifndef UNIVERSE_UTIL_H
#define UNIVERSE_UTIL_H

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
float randomFloat(float mn, float mx);

std::string readFile(const char* fileName);

#endif
