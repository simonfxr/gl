
uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;

out vec3 ecNormal;
out vec3 ecPosition;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);
    ecNormal = normalMatrix * normal;
    ecPosition = (mvMatrix * vec4(position, 1)).xyz;
}
