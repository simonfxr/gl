

uniform mat4 mvpMatrix;

in vec3 position;

void main() {
    gl_Position = vec4(position.x, position.z, 0, 1);
    /* gl_Position = mvpMatrix * vec4(position, 1); */
}
