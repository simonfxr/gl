#version 330

uniform mat4 mvpMatrix;

in vec3 vertex;
in vec3 normal;

out vec3 mcVertex;
out vec3 mcNormal;

void main()
{
    mcVertex = vertex;
    mcNormal = normal;
    
    gl_Position = mvpMatrix * vec4(vertex, 1);
}

