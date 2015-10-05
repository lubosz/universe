/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#ifndef SRC_SIMULATOR_H_
#define SRC_SIMULATOR_H_

#include "GL/gl3w.h"

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

// issue with using cl_float4 from cl_platform.h
// http://www.khronos.org/message_boards/viewtopic.php?f=28&t=1848
// typedef cl_float cl_float4
// __attribute__ ((__vector_size__ (16), __may_alias__));
typedef struct Vec4 {
    float x, y, z, w;
    Vec4() {}
    // convenience functions
    Vec4(float xx, float yy, float zz, float ww):
        x(xx),
        y(yy),
        z(zz),
        w(ww)
    {}
    void set(float xx, float yy, float zz, float ww = 1.) {
        x = xx;
        y = yy;
        z = zz;
        w = ww;
    }
} Vec4;
//} Vec4 __attribute__((aligned(16)));

class Simulator {
 public:
    std::vector<cl::Memory> cl_vbos;
    cl::Buffer velocityBuffer;
    cl::Buffer gravityBuffer;
    cl::Buffer initialPositionBuffer;
    cl::Buffer initivalVelocityBuffer;

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
            std::vector<Vec4> pos,
            std::vector<Vec4> vel,
            std::vector<Vec4> color,
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
