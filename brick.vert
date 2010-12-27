#version 330

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 ecPosition;
flat out vec3 ecNormal;
out vec2 texCoord;

void main() {
    
    ecPosition = vec3(mvMatrix * vec4(vertex, 1));
    ecNormal = normalMatrix * normal;
    
    if (normal.z != 0)
        texCoord = vertex.xy;
    else if (normal.y != 0)
        texCoord = vertex.xz;
    else
        texCoord = vertex.zy;

    gl_Position = mvpMatrix * vec4(vertex, 1);
}
