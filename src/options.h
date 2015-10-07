#ifndef OPTIONS_H
#define OPTIONS_H

// Window
const int window_width = 640;
const int window_height = 480;

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

#define NUM_PARTICLES 10000

// Speed
const float slowDt = 1000.0f;
const float fastDt = 10 * slowDt;

const float bigMass = 20000;

#endif // OPTIONS_H

