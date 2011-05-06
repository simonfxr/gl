

uniform mat4 mvpMatrix;

in vec4 position;
in vec3 normal;

void main() {
    gl_Position = mvpMatrix * position;
}
