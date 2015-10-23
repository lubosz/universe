/* Universe
 *
 * The MIT License (MIT)
 *
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


#ifndef OPTIONS_H
#define OPTIONS_H

// Window
const int window_width = 1280;
const int window_height = 720;

const std::string title = "Universe Simulator";

// Camera Position
const float initialScrollPosition = -2.5;
const float initialTheta = 1.8;
const float initialPhi = 0;

// Camera
const float fov = 45.0f;

// Input Speed
const float scrollSpeed = 0.05;
const float rotationSpeed = 0.002;

// Simulation

#define NUM_PARTICLES 15000

// Speed
const float slowDt = 100.0f;
const float fastDt = 10 * slowDt;

const float bigMass = 1;

#endif // OPTIONS_H

