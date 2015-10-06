#version 400

in vec4 color;
in float massColor;
uniform sampler2D cloud;

out vec4 frag_colour;

void main () {

  vec4 starColor = mix(vec4(.9,.1,.1,1), vec4(.1,.1,.9,1), color.x);
  starColor = mix(starColor, vec4(1,1,1,1), clamp(massColor,0,1));

  frag_colour = starColor * texture(cloud, vec2(gl_PointCoord.x, gl_PointCoord.y));
  //frag_colour = vec4(1,0,0,1);
};
