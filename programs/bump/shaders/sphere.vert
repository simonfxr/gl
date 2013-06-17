
uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform vec3 ecLight;

in vec3 position;
in vec3 normal;

out vec3 tsLight;
out vec3 tsEye;
out vec3 ecNormal;
flat out vec3 ecNormal0;

void main() {
    gl_Position = mvpMatrix * vec4(position, 1);

    ecNormal = normalize(normalMatrix * normal);
    ecNormal0 = ecNormal;
    
    vec3 ecPosition = (mvMatrix * vec4(position, 1)).xyz;
    vec3 ecLightDir = normalize(ecLight - ecPosition);
    vec3 ecEye = normalize(- ecPosition);

    float cos_theta = ecNormal.z;
    float sin_theta = sqrt(ecNormal.x * ecNormal.x + ecNormal.y * ecNormal.y);
    float cos_phi, sin_phi;
    if (sin_theta < 0.001) {
        cos_phi = 0;
        sin_phi = 0;
    } else {
        float d = 1 / sin_theta;
        cos_phi = ecNormal.x / d;
        sin_phi = ecNormal.y / d;
    }

    vec3 ecTangent = normalize(vec3(cos_theta * cos_phi, cos_theta * sin_phi, - sin_theta));
    vec3 ecBinormal = cross(ecNormal, ecTangent);
    mat3 iTBN = transpose(mat3(ecTangent, ecBinormal, ecNormal));

    tsLight = iTBN * ecLightDir;
    tsEye = iTBN * ecEye;
}
