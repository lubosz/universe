/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#include <stdio.h>
#include <string>
#include <iostream>

#include "Simulator.h"
#include "Renderer.h"
#include "util.h"
#include <GL/glx.h>

using std::string;

static std::string deviceTypeToString(int type) {
    switch (type) {
    case CL_DEVICE_TYPE_DEFAULT:
        return "CL_DEVICE_TYPE_DEFAULT";
    case CL_DEVICE_TYPE_GPU:
        return "CL_DEVICE_TYPE_GPU";
    case CL_DEVICE_TYPE_CPU:
        return "CL_DEVICE_TYPE_CPU";
    default:
        return std::to_string(type);
    }
}

Simulator::Simulator() {
    std::vector<cl::Platform> platforms;
    cl::Platform currentPlatform;
    err = cl::Platform::get(&platforms);

    if (err != CL_SUCCESS) {
        printf("Error getting platforms: %s\n", oclErrorString(err));
        exit(0);
    }

    printf("Available Platforms (%ld):\n", platforms.size());
    for (auto platform : platforms) {
        string platformName;
        std::vector<cl::Device> devices;
        platform.getInfo(CL_PLATFORM_NAME, &platformName);
        try {
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        } catch (cl::Error er) {
            printf("ERROR: Could not get Devices for Platform '%s'. %s(%s)\n",
                   platformName.c_str(), er.what(), oclErrorString(er.err()));
            continue;
        }
        printf("=== %s (%ld Devices) ===\n", platformName.c_str(), devices.size());
        for (auto device : devices) {
            string type = deviceTypeToString(device.getInfo<CL_DEVICE_TYPE>());
            string deviceName = device.getInfo<CL_DEVICE_NAME>();
            printf("%s: %s\n", type.c_str(), deviceName.c_str());

            //if (deviceName.compare("Intel(R) Core(TM) i7-4550U CPU @ 1.50GHz") == 0) {
            if (deviceName.compare("Intel(R) HD Graphics Haswell Ultrabook GT3 Mobile") == 0) {
                currentPlatform = platform;
                currentDevice = device;
            }

        }
    }

    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glXGetCurrentContext()),
        CL_GLX_DISPLAY_KHR,
        reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()),
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>((currentPlatform)()),
        0
    };
    // cl_context cxGPUContext =
    // clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
    try {
        context = cl::Context(CL_DEVICE_TYPE_GPU, props);
    } catch (cl::Error er) {
        printf("ERROR: Could not create CL context. %s(%s) %d\n", er.what(), oclErrorString(er.err()), er.err());
    }

    // create the command queue we will use to execute OpenCL commands
    try {
        queue = cl::CommandQueue(context, currentDevice, 0, &err);
    } catch (cl::Error er) {
        printf("ERROR: %s(%d)\n", er.what(), er.err());
    }
    gravities = (float *)malloc(particleCount*sizeof(float));
    dt = 1000.0f;
}

Simulator::~Simulator() {}


void Simulator::loadProgram(std::string kernel_source) {
    // Program Setup
    int pl = kernel_source.size();
    try {
        cl::Program::Sources source(
                    1, std::make_pair(kernel_source.c_str(), pl));
        program = cl::Program(context, source);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    std::vector<cl::Device> devices;
    devices.push_back(currentDevice);

    try {
        // err = program.build(devices,
        // "-cl-nv-verbose -cl-nv-maxrregcount=100");
        err = program.build(devices);
    }
    catch (cl::Error er) {
        printf("program.build: %s\n", oclErrorString(er.err()));
        exit(0);
    }
    std::cout << "Build Status: "
              << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0])
              << std::endl << "Build Options:\t"
              << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0])
              << std::endl << "Build Log:\t "
              << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
              << std::endl;
}

void Simulator::loadData(std::vector<Vec4> pos,
        std::vector<Vec4> vel,
        std::vector<Vec4> col,
        std::vector<float> mass) {
    // store the number of particles and the size in bytes of our arrays
    particleCount = pos.size();
    array_size = particleCount * sizeof(Vec4);
    // create VBOs (defined in util.cpp)
    positionVBO = Renderer::createVBO(
                &pos[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    colorVBO = Renderer::createVBO(
                &col[0], array_size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    massVBO = Renderer::createVBO(
                mass.data(), particleCount * sizeof(GLfloat),
                GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    // make sure OpenGL is finished before we proceed
    glFinish();
    printf("gl interop!\n");
    // create OpenCL buffer from GL VBO
    cl_vbos.push_back(cl::BufferGL(
                          context, CL_MEM_READ_WRITE, positionVBO, &err));
    cl_vbos.push_back(cl::BufferGL(
                          context, CL_MEM_READ_WRITE, colorVBO, &err));

    cl_vbos.push_back(cl::BufferGL(
                          context, CL_MEM_READ_WRITE, massVBO, &err));

    // create the OpenCL only arrays
    velocityBuffer =
            cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
    initialPositionBuffer =
            cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
    initivalVelocityBuffer =
            cl::Buffer(context, CL_MEM_WRITE_ONLY, array_size, NULL, &err);
    //gravityBuffer =
    //        cl::Buffer(context, CL_MEM_WRITE_ONLY, particleCount * sizeof(float), NULL, &err);
    printf("Pushing data to the GPU\n");
    // push our CPU arrays to the GPU
    // data is tightly packed in std::vector
    // starting with the adress of the first element
    err = queue.enqueueWriteBuffer(
                velocityBuffer, CL_TRUE, 0, array_size,
                &vel[0], NULL, &event);
    err = queue.enqueueWriteBuffer(
                initialPositionBuffer, CL_TRUE, 0, array_size,
                &pos[0], NULL, &event);
    err = queue.enqueueWriteBuffer(
                initivalVelocityBuffer, CL_TRUE, 0, array_size,
                &vel[0], NULL, &event);
    queue.finish();
}

void Simulator::initKernel() {
    try {
        kernel = cl::Kernel(program, "vortex", &err);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    try {
        err = kernel.setArg(0, cl_vbos[0]);
        err = kernel.setArg(1, cl_vbos[1]);
        err = kernel.setArg(2, cl_vbos[2]);
        err = kernel.setArg(3, velocityBuffer);
        err = kernel.setArg(4, initialPositionBuffer);
        err = kernel.setArg(5, initivalVelocityBuffer);
        //err = kernel.setArg(6, gravityBuffer);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }
    // Wait for the command queue to finish these commands before proceeding
    queue.finish();
}


void Simulator::runKernel() {
    // this will update our system by calculating new velocity
    // and updating the positions of our particles
    // Make sure OpenGL is done using our VBOs
    glFinish();
    // map OpenGL buffer object for writing from OpenCL
    // this passes in the vector of VBO buffer objects (position and color)
    err = queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &event);

    // pass in the timestep
    kernel.setArg(6, dt);
    // execute the kernel
    err = queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(particleCount),
                cl::NullRange, NULL, &event);
    // printf("clEnqueueNDRangeKernel: %s\n", oclErrorString(err));
    queue.finish();
/*
    queue.enqueueReadBuffer(gravityBuffer,false,0,particleCount*sizeof(float),gravities,NULL,NULL);
    for (int i=0; i < particleCount; i++) {
            std::cout << "gravities[" <<  i << "] = " << gravities[i] << "\n";
    }
*/
    // Release the VBOs so OpenGL can play with them
    err = queue.enqueueReleaseGLObjects(&cl_vbos, NULL, &event);
    queue.finish();
}

const char* Simulator::oclErrorString(cl_int error) {
    static const char* errorString[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "CL_MISALIGNED_SUB_BUFFER_OFFSET",
        "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
        "CL_COMPILE_PROGRAM_FAILURE",
        "CL_LINKER_NOT_AVAILABLE",
        "CL_LINK_PROGRAM_FAILURE",
        "CL_DEVICE_PARTITION_FAILED",
        "CL_KERNEL_ARG_INFO_NOT_AVAILABLE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
        "CL_INVALID_PROPERTY",
        "CL_INVALID_IMAGE_DESCRIPTOR",
        "CL_INVALID_COMPILER_OPTIONS",
        "CL_INVALID_LINKER_OPTIONS",
        "CL_INVALID_DEVICE_PARTITION_COUNT"
    };
    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);
    const int index = -error;
    return (index >= 0 && index < errorCount) ? errorString[index] : "";
}
