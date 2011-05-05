#version 330

uniform mat4 projectionMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 ecNormal;
out vec3 ecPosition;
out vec2 fragTexCoord;

void main() {
    fragTexCoord = texCoord;
    ecNormal = normalMatrix * normal;
    vec4 ecVert = mvMatrix * vec4(position, 1.0);
    ecPosition = vec3(ecVert) / ecVert.w;
    gl_Position = projectionMatrix * vec4(ecPosition, 1.0);
}
