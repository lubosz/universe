#version 400

in vec4 vp;
in vec4 cp;
uniform mat4 mvp;
out vec4 color;

void main () {
  color = cp;
  gl_Position = mvp * vec4 (vp);
};
