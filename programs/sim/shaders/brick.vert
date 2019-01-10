
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

SL_in vec4 vertex;
SL_in vec3 normal;

SL_out vec3 ecPosition;
SL_out vec3 ecNormal;
SL_out vec2 texCoord;

void
main()
{
    ecPosition = vec3(mvMatrix * vertex);
    ecNormal = normalMatrix * normal;

    if (normal.z != 0)
        texCoord = vertex.xy;
    else if (normal.y != 0)
        texCoord = vertex.xz;
    else
        texCoord = vertex.zy;

    gl_Position = mvpMatrix * vertex;
}
