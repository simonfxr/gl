#version 330

uniform vec4 color;
uniform mat4 rotation;

attribute vec4 position;

varying vec4 tone;

void main() {
     gl_Position = rotation * vec4(position.xy, 0, 1);

     if (position.z == 1)
        tone = vec4(1, 0, 0, 1);
     else if (position.z == 2)
        tone = vec4(0, 1, 0, 1);
     else if (position.z == 3)
        tone = vec4(0, 0, 1, 1);
     else
        tone = color;
}
