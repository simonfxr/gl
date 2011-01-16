#version 330

#include "sim_shading.h"

const vec3 BrickColor  = vec3(0.8, 0, 0);
const vec3 MortarColor = vec3(0.6);
const vec2 BrickSize   = vec2(0.06, 0.03);
const vec2 BrickPct    = vec2(0.9, 0.85);

const float Shininess  = 16;

uniform vec3 ecLight;
uniform vec3 ecSpotDirection;
uniform float spotAngle;

in vec3 ecPosition;
in vec3 ecNormal;
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
    
    fragColor = shade(ecLight, ecPosition, normalize(ecNormal), Shininess, vec4(color, 1));
}
