#include <stdio.h>
#include <string>
#include <iostream>

#include "Simulator.h"
#include "util.h"
#include <GL/glx.h>


Simulator::Simulator()
{
    printf("Initialize OpenCL object and context\n");
    //setup devices and context
    std::vector<cl::Platform> platforms;
    err = cl::Platform::get(&platforms);
    printf("cl::Platform::get(): %s\n", oclErrorString(err));
    printf("platforms.size(): %ld\n", platforms.size());

    deviceUsed = 0;
    err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    printf("getDevices: %s\n", oclErrorString(err));
    printf("devices.size(): %ld\n", devices.size());
    int t = devices.front().getInfo<CL_DEVICE_TYPE>();
    printf("type: device: %d CL_DEVICE_TYPE_GPU: %d \n", t, CL_DEVICE_TYPE_GPU);

    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(glXGetCurrentContext()),
        CL_GLX_DISPLAY_KHR, reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()),
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>((platforms[0])()),
        0
    };
    //cl_context cxGPUContext = clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
    try{
        context = cl::Context(CL_DEVICE_TYPE_GPU, props);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    //create the command queue we will use to execute OpenCL commands
    try{
        queue = cl::CommandQueue(context, devices[deviceUsed], 0, &err);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%d)\n", er.what(), er.err());
    }

}

Simulator::~Simulator() {}


void Simulator::loadProgram(std::string kernel_source)
{
    // Program Setup
    int pl;
    //size_t program_length;
    printf("load the program\n");
    
    pl = kernel_source.size();
    printf("kernel size: %d\n", pl);
    //printf("kernel: \n %s\n", kernel_source.c_str());
    try
    {
        cl::Program::Sources source(1,
                                    std::make_pair(kernel_source.c_str(), pl));
        program = cl::Program(context, source);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    printf("build program\n");
    try
    {
        //err = program.build(devices, "-cl-nv-verbose -cl-nv-maxrregcount=100");
        err = program.build(devices);
    }
    catch (cl::Error er) {
        printf("program.build: %s\n", oclErrorString(er.err()));
        //if(err != CL_SUCCESS){
    }
    printf("done building program\n");
    std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) << std::endl;
    std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0]) << std::endl;
    std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;

}

void Simulator::loadData(std::vector<Vec4> pos, std::vector<Vec4> vel, std::vector<Vec4> col)
{
    //store the number of particles and the size in bytes of our arrays
    particleCount = pos.size();
    array_size = particleCount * sizeof(Vec4);
    //create VBOs (defined in util.cpp)
    positionVBO = createVBO(&pos[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    colorVBO = createVBO(&col[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

    //make sure OpenGL is finished before we proceed
    glFinish();
    printf("gl interop!\n");
    // create OpenCL buffer from GL VBO
    cl_vbos.push_back(cl::BufferGL(context, CL_MEM_READ_WRITE, positionVBO, &err));
    //printf("v_vbo: %s\n", oclErrorString(err));
    cl_vbos.push_back(cl::BufferGL(context, CL_MEM_READ_WRITE, colorVBO, &err));
    //we don't need to push any data here because it's already in the VBO


    //create the OpenCL only arrays
    velocityBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
    initialPositionBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
    initivalVelocityBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);

    printf("Pushing data to the GPU\n");
    //push our CPU arrays to the GPU
    //data is tightly packed in std::vector starting with the adress of the first element
    err = queue.enqueueWriteBuffer(velocityBuffer, CL_TRUE, 0, array_size, &vel[0], NULL, &event);
    err = queue.enqueueWriteBuffer(initialPositionBuffer, CL_TRUE, 0, array_size, &pos[0], NULL, &event);
    err = queue.enqueueWriteBuffer(initivalVelocityBuffer, CL_TRUE, 0, array_size, &vel[0], NULL, &event);
    queue.finish();
}

void Simulator::initKernel()
{
    //initialize our kernel from the program
    try{
        kernel = cl::Kernel(program, "vortex", &err);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    //set the arguements of our kernel
    try
    {
        err = kernel.setArg(0, cl_vbos[0]); //position vbo
        err = kernel.setArg(1, cl_vbos[1]); //color vbo
        err = kernel.setArg(2, velocityBuffer);
        err = kernel.setArg(3, initialPositionBuffer);
        err = kernel.setArg(4, initivalVelocityBuffer);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }
    //Wait for the command queue to finish these commands before proceeding
    queue.finish();
}


void Simulator::runKernel()
{
    //this will update our system by calculating new velocity and updating the positions of our particles
    //Make sure OpenGL is done using our VBOs
    glFinish();
    // map OpenGL buffer object for writing from OpenCL
    //this passes in the vector of VBO buffer objects (position and color)
    err = queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &event);
    //printf("acquire: %s\n", oclErrorString(err));
    queue.finish();

    float dt = .01f;
    kernel.setArg(5, dt); //pass in the timestep
    //execute the kernel
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(particleCount), cl::NullRange, NULL, &event);
    //printf("clEnqueueNDRangeKernel: %s\n", oclErrorString(err));
    queue.finish();

    //Release the VBOs so OpenGL can play with them
    err = queue.enqueueReleaseGLObjects(&cl_vbos, NULL, &event);
    //printf("release gl: %s\n", oclErrorString(err));
    queue.finish();

}
