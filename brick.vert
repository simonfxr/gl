#version 330

uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 mcVertex;
flat out vec3 eyeNormal;
out vec2 texCoord;

void main()
{
    mcVertex = vertex;
    eyeNormal = normalMatrix * normal;
    
    if (normal.z != 0)
        texCoord = vertex.xy;
    else if (normal.y != 0)
        texCoord = vertex.xz;
    else
        texCoord = vertex.zy;

    gl_Position = mvpMatrix * vec4(vertex, 1);
}

