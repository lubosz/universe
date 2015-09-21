#ifndef SIMULATOR_H
#define SIMULATOR_H

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

// issue with using cl_float4 from cl_platform.h
// http://www.khronos.org/message_boards/viewtopic.php?f=28&t=1848
// typedef cl_float cl_float4 __attribute__ ((__vector_size__ (16), __may_alias__));
typedef struct Vec4
{
    float x,y,z,w;
    Vec4(){}
    //convenience functions
    Vec4(float xx, float yy, float zz, float ww):
        x(xx),
        y(yy),
        z(zz),
        w(ww)
    {}
    void set(float xx, float yy, float zz, float ww=1.) {
        x = xx;
        y = yy;
        z = zz;
        w = ww;
    }
} Vec4;
//} Vec4 __attribute__((aligned(16)));

class Simulator {
public:

    //These are arrays we will use in this tutorial
    std::vector<cl::Memory> cl_vbos;  //0: position vbo, 1: color vbo
    cl::Buffer velocityBuffer;  //particle velocities
    cl::Buffer initialPositionBuffer;  //want to have the start points for reseting particles
    cl::Buffer initivalVelocityBuffer;  //want to have the start velocities for reseting particles

    int positionVBO;   //position vbo
    int colorVBO;   //colors vbo
    int particleCount;    //the number of particles
    size_t array_size; //the size of our arrays num * sizeof(Vec4)

    //default constructor initializes OpenCL context and automatically chooses platform and device
    Simulator();
    //default destructor releases OpenCL objects and frees device memory
    ~Simulator();

    //load an OpenCL program from a string
    void loadProgram(std::string kernel_source);
    //setup the data for the kernel
    void loadData(std::vector<Vec4> pos, std::vector<Vec4> vel, std::vector<Vec4> color);
    //these are implemented in part1.cpp (in the future we will make these more general)
    void initKernel();
    //execute the kernel
    void runKernel();

    //    real coders bare all
    //    private:

    unsigned int deviceUsed;
    std::vector<cl::Device> devices;

    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Kernel kernel;

    //debugging variables
    cl_int err;
    ///cl_event event;
    cl::Event event;

    static const char* oclErrorString(cl_int error);
};

#endif
