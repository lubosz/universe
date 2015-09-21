CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(universe)

FIND_PACKAGE(OpenCL)
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(GLEW)
FIND_PACKAGE(glm)
FIND_PACKAGE(glfw3)

INCLUDE_DIRECTORIES( 
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${GLFW3_INCLUDE_PATH}
    ${GLM_INCLUDE_PATH}
)

function(add_cxxflag FLAG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}" PARENT_SCOPE)
endfunction(add_cxxflag)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    #Debug
    message("On Debug Build")
    add_cxxflag("-O0")
    add_cxxflag("-g3")
    add_definitions(-D_DEBUG)
else()
    #Release
    message("On Release Build")
    add_cxxflag("-O3")
    add_definitions(-D_NDEBUG)
endif()

#Set c++1
add_cxxflag("-std=c++14")

ADD_EXECUTABLE(universe
  src/main.cpp
  src/cll.cpp
  src/part2.cpp
  src/util.cpp)

TARGET_LINK_LIBRARIES (universe
   ${OPENGL_LIBRARIES}
   ${GLEW_LIBRARY}
   ${OpenCL_LIBRARY}
   ${GLFW3_LIBRARY}
)
