
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;
in vec4 color;

out vec3 ecNormal;
out vec3 ecPosition;
flat out vec4 vColor;

void main() {
    vColor = color;
    ecNormal = normalMatrix * normal;
    ecPosition = vec3(mvMatrix * vec4(position, 1.0));
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
