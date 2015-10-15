/* Universe
 *
 * The MIT License (MIT)
 *
 * Copyright 2009-2011 Ian 'enjalot' Johnson
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

#ifndef SRC_SIMULATOR_H_
#define SRC_SIMULATOR_H_

#include "GL/gl3w.h"
#include <glm/glm.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

class Simulator {
 public:
    std::vector<cl::Memory> cl_vbos;
    cl::Buffer velocityBuffer;
    cl::Buffer gravityBuffer;

    int positionVBO;
    int colorVBO;
    int massVBO;
    int particleCount;
    float* gravities;
    size_t array_size;
    float dt;

    Simulator();
    ~Simulator();

    void loadProgram(std::string kernel_source);
    void loadData(
            std::vector<glm::vec4> pos,
            std::vector<glm::vec4> vel,
            std::vector<glm::vec4> color,
            std::vector<float> mass);
    void initKernel();
    void runKernel();

    cl::Device currentDevice;

    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Kernel kernel;

    cl_int err;
    cl::Event event;

    static const char* oclErrorString(cl_int error);
};

#endif  // SRC_SIMULATOR_H_
