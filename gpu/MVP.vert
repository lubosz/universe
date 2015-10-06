#version 400

in vec4 vp;
in vec4 cp;
in float mp;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
out vec4 color;
out float massColor;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

void main () {
  color = cp;
  vec4 position = viewMatrix * vec4 (vp);
  float distance = -position.z;
  gl_Position = projectionMatrix * position;
  gl_PointSize = 2 * mp / distance;
  massColor = mp / 2000;
};
