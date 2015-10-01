__constant float GRAVITY = 0.000000000066742;
__constant float mass = 1.0;


__kernel void vortex(
  __global float4* pos,
  __global float4* color,
  __global float4* vel,
  __global float4* pos_gen,
  __global float4* vel_gen,
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

    //we've stored the life in the fourth component of our velocity array
    float life = vel[i].w;
    //decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
    life -= dt;
    //if the life is 0 or less we reset the particle's values back to the original values and set life to 1
    if(life <= 0)
    {
        p = pos_gen[i];
        v = vel_gen[i];
        life = 10.0;
    }


    float4 force = (float4)(0, 0, 0, 1);
    for (int j = 0; j < particle_count; j++) {
      float4 distance = pos[j] - p;

      if (length(distance) != 0) {
        force += normalize(distance) * GRAVITY * 1.0 / (pow(length(distance),2));
        color[i].y = 1.0;
      }

    }

    v += 10* force;


    //we use a first order euler method to integrate the velocity and position (i'll expand on this in another tutorial)
    //update the velocity to be affected by "gravity" in the z direction
    //v.z -= 9.8*dt;
    //update the position with the new velocity
    p += v*dt;
    //store the updated life in the velocity array
    v.w = life;

    //update the arrays with our newly computed values
    pos[i] = p;
    vel[i] = v;

    //you can manipulate the color based on properties of the system
    //here we adjust the alpha
    //color[i].w = life;

}
