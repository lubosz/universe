#pragma OPENCL EXTENSION cl_khr_fp64 : enable

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

        // Ignore 0 distances
        if (length(distance) <= 0)
            continue;

        // Calculate force with minimum distance threshold
        if (length(distance) > 0.1) {
          float acceleration = GRAVITY * masses[j] / (pow(length(distance),2));
          accelerationDirection += normalize(distance) * acceleration;
        }

        // Merge small particle into big if distance is short enough
        if (
            //length(distance) < (masses[j]+masses[i]) * 0.00015 &&
            length(distance) < 0.01 &&
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
