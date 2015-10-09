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

    if (mass == 0)
      return;

      for (int j = 0; j < particle_count; j++) {
        if (masses[j] == 0  || j == i)
          continue;
        float4 distance = pos[j] - p;

        if (length(distance) <= 0)
            continue;

        if (length(distance) > 0.1) {
          float acceleration = GRAVITY * masses[j] / (pow(length(distance),2));
          accelerationDirection += normalize(distance) * acceleration;
        }

        if (
            //length(distance) < (masses[j]+masses[i]) * 0.00015 &&
            length(distance) < 0.01 &&
            masses[i] < masses[j]) {
                  masses[j] += masses[i];
                  vel[j] += vel[i] * masses[i] / masses[j];
                  masses[i] = 0;
                  return;
        }
      }

    v += accelerationDirection*dt;
    v.w = 1;

    p += v*dt;
    p.w = 1;

    pos[i] = p;
    vel[i] = v;
}
