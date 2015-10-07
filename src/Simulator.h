/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
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
