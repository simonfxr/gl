
const float Ambient = 0.3;
const float Diffuse = 0.9;
const float Specular = 0.3;

in vec3 tsLight;
in vec3 tsEye;
in vec3 ecNormal;
flat in vec3 ecNormal0;

out vec4 fragColor;

void main() {

    vec3 off = normalize(ecNormal) - ecNormal0;

    vec3 light = normalize(tsLight);
    vec3 eye = normalize(tsEye);
    vec3 normal = vec3(0, 0, 1) - off;

    float d = clamp(dot(normal, light), 0, 1);
    /* float s = clamp(dot(eye, reflect(lightDir, normal)), 0, 1); */
    /* s = pow(s, 200); */

    /* vec3 albedo = vec3(1, 0, 0) * texCoord.t + vec3(0, 0, 1) * (1 - texCoord.t); */
    /* vec3 lightColor = vec3(1, 1, 1); */

    fragColor = vec4(d, 0, 0, 1);
}
