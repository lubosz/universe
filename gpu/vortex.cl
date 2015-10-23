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


// #pragma OPENCL EXTENSION cl_khr_fp64 : enable

__constant float GRAVITY = 0.000000000066742;

__kernel void vortex(
  __global float4* pos,
  __global float4* color,
  __global float* masses,
  __global float4* vel,
  float dt)
{
    unsigned int i = get_global_id(0);
    unsigned int particle_count = get_global_size(0);

    float4 p = pos[i];
    float4 v = vel[i];
    float mass = masses[i];

    float4 accelerationDirection = (float4)(0, 0, 0, 0);

    // Ignore deleted particles
    if (mass == 0)
      return;

    // Calculate gravitational force for all particles
    for (int j = 0; j < particle_count; j++) {
        // Ignore deleted masses, ignore gravitation to self
        if (masses[j] == 0  || j == i)
          continue;

        float4 distance = pos[j] - p;
        float qdistance = dot(distance, distance);

        // Ignore 0 distances
        if (qdistance <= 0)
            continue;

        if (qdistance > 0.01) {
          float acceleration = GRAVITY * masses[j] / qdistance;
          accelerationDirection += normalize(distance) * acceleration;
        }

        // Merge small particle into big if distance is short enough
        if (
            //length(distance) < (masses[j]+masses[i]) * 0.00015 &&
            qdistance < 0.0001 &&
            masses[i] < masses[j]) {
                  masses[j] += masses[i];
                  // Use small particle velocity on big
                  vel[j] += vel[i] * masses[i] / masses[j];
                  // Delete small particle
                  masses[i] = 0;
                  return;
        }
    }

    // Calculate new velocity with acceleration
    v += accelerationDirection*dt;
    v.w = 1;

    // Calculate new position with velocity
    p += v*dt;
    p.w = 1;

    // Update positions and velocities
    pos[i] = p;
    vel[i] = v;
}
