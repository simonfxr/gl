#version 330

uniform vec3 ecLight;
uniform vec4 color;
uniform float shininess;

const float Ambient = 0.7;
const float Diffuse = 0.2;
const float Specular = 0.1;

in vec3 ecPosition;
in vec3 ecNormal;

out vec4 fragColor;

void main() {
    
    vec3 light = normalize(ecLight - ecPosition);
    vec3 eyeNormal = normalize(ecNormal);
    float ambientIntensity = max(0, dot(eyeNormal, light));

    vec3 spectator = normalize(-ecPosition);
    vec3 lightReflect = reflect(-light, eyeNormal);
    float specularIntensity = pow(max(0, dot(spectator, lightReflect)), shininess);

    vec3 baseColor = vec3(color);
    vec3 tone = baseColor * Diffuse +
                baseColor * Ambient * ambientIntensity +
                vec3(0.8) * Specular * specularIntensity;
    
    fragColor = vec4(tone, color.a);
}
