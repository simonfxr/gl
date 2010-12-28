#version 330

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec4 position;
in vec3 normal;

out vec3 ecPosition;
out vec3 ecNormal;

void main() {
    ecPosition = vec3(mvMatrix * position);
    ecNormal = normalMatrix * normal;
    gl_Position = mvpMatrix * position;
}
