
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;

out vec3 ecNormal;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);
    ecNormal = normalMatrix * normal;
}
