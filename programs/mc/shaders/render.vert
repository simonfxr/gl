
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;

out vec3 ecNormal;
out vec3 ecPosition;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);
    ecPosition = (mvMatrix * vec4(position, 1)).xyz;
    ecNormal = normalMatrix * normal;
}
