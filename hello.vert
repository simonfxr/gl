#version 110

uniform mat4 rotation;

attribute vec4 position;

void main() {
     gl_Position = rotation * position;
}
