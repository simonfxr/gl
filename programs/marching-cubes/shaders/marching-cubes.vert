uniform mat4 modelMatrix;

in vec3 position;
out vec3 vTexCoord;

void main() {
    vTexCoord = position;
    gl_Position = modelMatrix * vec4(position, 1);
}
