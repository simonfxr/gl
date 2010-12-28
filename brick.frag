#version 330

const vec3 BrickColor  = vec3(0.8, 0, 0);
const vec3 MortarColor = vec3(0.6);
const vec2 BrickSize   = vec2(0.06, 0.03);
const vec2 BrickPct    = vec2(0.9, 0.85);

const float Ambient = 0.7;
const float Diffuse = 0.2;
const float Specular = 0.1;

uniform vec3 ecLight;

in vec3 ecPosition;
flat in vec3 ecNormal;
in vec2 texCoord;

out vec4 fragColor;

vec3 textureSample(vec2 coord) {
    vec2 position = coord / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;
    
    position = fract(position);

    vec2 useBrick = step(position, BrickPct);
    
    return mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
}

void main() {
    
    vec3 color = textureSample(texCoord);

    vec3 light = normalize(ecLight - ecPosition);
    vec3 eyeNormal = normalize(ecNormal);
    float ambientIntensity = max(0, dot(eyeNormal, light));

    vec3 spectator = normalize(-ecPosition);
    vec3 lightReflect = reflect(-light, eyeNormal);
    float specularIntensity = pow(max(0, dot(spectator, lightReflect)), 16);

    color = color * Diffuse +
            color * Ambient * ambientIntensity +
            vec3(0.8) * Specular * specularIntensity;

    fragColor = vec4(color, 1);
}
