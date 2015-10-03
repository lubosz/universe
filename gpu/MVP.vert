#version 150

in vec4 vp;
in vec4 cp;
in float mp;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
out vec4 color;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

void main () {
  color = cp;
  vec4 position = modelMatrix * vec4 (vp);
  float distance = -position.z;
  gl_Position = projectionMatrix * position;
  gl_PointSize = 2 * mp / distance;
};
