#version 400

in vec4 vp;
in vec4 cp;
uniform mat4 mvp;
out vec4 color;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

void main () {
  color = cp;
  gl_Position = mvp * vec4 (vp);
  gl_PointSize = 6.0;
};
