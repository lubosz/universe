/*
 * Universe
 *
 * Copyright 2015 Lubosz Sarnecki <lubosz@gmail.com>
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <streambuf>
#include <iostream>

#include "util.h"

float randomFloat(float mn, float mx) {
    float r = random() / static_cast<float>(RAND_MAX);
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
