#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__constant float GRAVITY = 0.000000000000066742;


__kernel void vortex(
  __global float4* pos,
  __global float4* color,
  __global float* masses,
  __global float4* vel,
  __global float4* pos_gen,
  __global float4* vel_gen,
  //__global float* gravity,
  float dt)
{
    //get our index in the array
    unsigned int i = get_global_id(0);
    unsigned int particle_count = get_global_size(0);
    //copy position and velocity for this iteration to a local variable
    //note: if we were doing many more calculations we would want to have opencl
    //copy to a local memory array to speed up memory access (this will be the subject of a later tutorial)
    float4 p = pos[i];
    float4 v = vel[i];
    float mass = masses[i];

    float4 accelerationDirection = (float4)(0, 0, 0, 0);

    if (mass == 0)
      return;

    //if (p.x != 0 && p.z != 0)
      for (int j = 0; j < particle_count; j++) {
        if (masses[j] == 0)
          continue;
        float4 distance = pos[j] - p;
        if (length(distance) > 0.35) {
          float acceleration = GRAVITY * masses[j] / (pow(length(distance),2));
          accelerationDirection += normalize(distance) * acceleration;
        } else if (length(distance) < 0.01 && masses[i] < masses[j]) {
              masses[j] += masses[i];
              vel[j] += vel[i] * masses[i] / masses[j];
              masses[i] = 0;
        }


      }

    //gravity[i] = length(accelerationDirection);
    //gravity[i] = force;

    v += accelerationDirection * dt;
    v.w = 1;

    //we use a first order euler method to integrate the velocity and position (i'll expand on this in another tutorial)
    //update the velocity to be affected by "gravity" in the z direction
    //v.z -= 9.8*dt;
    //update the position with the new velocity
    p += v*dt;
    p.w = 1;
    //store the updated life in the velocity array


    //update the arrays with our newly computed values
    pos[i] = p;
    vel[i] = v;
}
