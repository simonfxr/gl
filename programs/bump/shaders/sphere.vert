#include "constants.h"

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 ecLight;

in vec3 position;
in vec3 tangent;
in vec3 binormal;
in vec2 uv;

out vec3 tsLight;
out vec3 tsEye;
out vec2 fragUV;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);

    fragUV = uv;
    float k = sign(uv.x);
    fragUV.x *= k;

    vec3 normal = k * cross(binormal, tangent);
    vec3 ecNormal = normalize(normalMatrix * normal);
    vec3 ecPosition = (mvMatrix * vec4(position, 1)).xyz;
    vec3 ecLightDir = normalize(ecLight - ecPosition);
    vec3 ecEye = normalize(- ecPosition);

    vec3 ecTangent = normalize(normalMatrix * tangent);
    vec3 ecBinormal = normalize(normalMatrix * binormal);

#define INVERSE_TBN(v) vec3(dot(v, ecTangent), dot(v, ecBinormal), dot(v, ecNormal))
    tsLight = INVERSE_TBN(ecLightDir);
    tsEye = INVERSE_TBN(ecEye);
#undef INVERSE_TBN
}
