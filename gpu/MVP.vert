#version 400

in vec4 vp;
in vec4 cp;
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
  gl_PointSize = 50.0;
  vec4 position = modelMatrix * vec4 (vp);
  gl_Position = projectionMatrix * position;
};
