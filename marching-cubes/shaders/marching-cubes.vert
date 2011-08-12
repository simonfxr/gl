
uniform mat4 modelMatrix;

in vec3 position;

void main() {
    gl_Position = modelMatrix * vec4(position);
}
