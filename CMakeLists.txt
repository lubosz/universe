CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
project(universe)

if(EXISTS ${CMAKE_SOURCE_DIR}/gl3w/gl3w_gen.py)
  message("gl3w_gen.py found")
else()
  message("gl3w_gen.py not found")
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND git submodule update --init gl3w)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/gl3w/include/GL/gl3w.h)
  message("gl3w_gen header generated.")
else()
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gl3w
    COMMAND python gl3w_gen.py)
endif()

set(glfw3_DIR /usr/lib/cmake/glfw)

set(CMAKE_AUTOMOC ON)

FIND_PACKAGE(OpenCL)
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(glm)
FIND_PACKAGE(glfw3)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5OpenGL)

QT5_WRAP_UI(UI_HEADERS src/window.ui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES( 
    ${OPENGL_INCLUDE_DIR}
    ${GLFW3_INCLUDE_PATH}
    ${GLM_INCLUDE_PATH}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/gl3w/include/
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

# add some warnings
add_cxxflag("-Wall")
add_cxxflag("-Wextra")
add_cxxflag("-Wuninitialized")
add_cxxflag("-Wold-style-cast")
add_cxxflag("-Woverloaded-virtual")
add_cxxflag("-Wformat=2")
add_cxxflag("-Wundef")
add_cxxflag("-Wpointer-arith")
add_cxxflag("-Wcast-qual")
add_cxxflag("-Wlogical-op")
add_cxxflag("-Woverflow")
add_cxxflag("-Wredundant-decls")



SET(SOURCES
  src/main.cpp
  src/Simulator.h
  src/Simulator.cpp
  src/Renderer.h
  src/Renderer.cpp
  src/util.h
  src/util.cpp
  src/myglwidget.cpp
  src/window.cpp
  gl3w/src/gl3w.c
)

ADD_EXECUTABLE(universe ${SOURCES})

TARGET_LINK_LIBRARIES (universe
   ${OPENGL_LIBRARIES}
   ${GLEW_LIBRARY}
   ${OpenCL_LIBRARY}
   ${GLFW3_LIBRARY}
   Qt5::Widgets
   Qt5::OpenGL
   dl
)

#check code stlye
if(EXISTS "/usr/bin/python2")
    set(PYTHON2 "python2")
else()
    set(PYTHON2 "python")
endif()

set(IGNORE "-readability/streams,-build/include")

if(EXISTS ${CMAKE_SOURCE_DIR}/cpplint.py)
      message("cpplint.py found")
else()
      execute_process(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND wget https://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
      )
endif()

add_custom_target(lint
    COMMAND ${PYTHON2} cpplint.py --filter=${IGNORE} ${SOURCES}
)
