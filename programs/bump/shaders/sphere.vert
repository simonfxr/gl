
uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 ecLight;

in vec3 position;
in vec3 normal;

out vec3 tsLight;
out vec3 tsEye;
out vec2 uv;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);

    vec3 ecNormal = normalize(normalMatrix * normal);
    vec3 ecPosition = (mvMatrix * vec4(position, 1)).xyz;
    vec3 ecLightDir = normalize(ecLight - ecPosition);
    vec3 ecEye = normalize(- ecPosition);

    float cos_theta = normal.z;
    float sin_theta = sqrt(normal.x * normal.x + normal.y * normal.y);
    float cos_phi, sin_phi;
    if (sin_theta < 0.001) {
        cos_phi = 0;
        sin_phi = 0;
    } else {
        float d = 1 / sin_theta;
        cos_phi = normal.x / d;
        sin_phi = normal.y / d;
    }

    vec3 ecTangent = normalMatrix * normalize(vec3(cos_theta * cos_phi, cos_theta * sin_phi, - sin_theta));
    vec3 ecBinormal = cross(ecNormal, ecTangent);
    mat3 iTBN = transpose(mat3(ecTangent, ecBinormal, ecNormal));

    uv.x = asin(normal.x) / 3.14159625358 + 0.5;
    uv.y = asin(normal.y) / 3.14159625358 + 0.5;

#define INVERSE_TBN(v) vec3(dot(v, ecTangent), dot(v, ecBinormal), dot(v, ecNormal))
    tsLight = INVERSE_TBN(ecLightDir);
    tsEye = INVERSE_TBN(ecEye);
#undef INVERSE_TBN
}
