#version 150

in vec4 color;
uniform sampler2D cloud;

out vec4 frag_colour;

void main () {
  frag_colour = texture(cloud, vec2(gl_PointCoord.x, gl_PointCoord.y));
  //frag_colour = vec4(1,0,0,1);
};
