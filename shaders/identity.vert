

uniform mat4 mvpMatrix;

in vec3 position;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);
}
