#include "constants.h"

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;
in float tri_id;

out vec3 ecNormal;
out vec3 N;
out vec3 ecPosition;
out vec3 color;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);

    ecNormal = normalize(normalMatrix * normal);
    N = normal;
    ecPosition = (mvMatrix * vec4(position, 1)).xyz;
    color.r = float(gl_VertexID / 5) * (1/5.);
    color.g = float(gl_VertexID % 5) * (1/5.);
    color.b = 0;

/*     fragUV = uv; */
/*     float k = sign(uv.x); */
/*     fragUV.x *= k; */

/*     N = normal; */
/*     /\* N = normalize(k * cross(binormal, tangent)); *\/ */

/*     vec3 ecNormal = normalize(normalMatrix * N); */
/*     vec3 ecPosition = (mvMatrix * vec4(position, 1)).xyz; */
/*     vec3 ecLightDir = normalize(ecLight - ecPosition); */
/*     vec3 ecEye = normalize(- ecPosition); */

/*     vec3 ecTangent = normalize(normalMatrix * tangent); */
/*     vec3 ecBinormal = normalize(normalMatrix * binormal); */

/* #define INVERSE_TBN(v) vec3(dot(v, ecTangent), dot(v, ecBinormal), dot(v, ecNormal)) */
/*     tsLight = INVERSE_TBN(ecLightDir); */
/*     tsEye = INVERSE_TBN(ecEye); */
/* #undef INVERSE_TBN */
}
