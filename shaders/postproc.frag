#version 330

#include "gamma.h"

uniform sampler3D textures;
uniform float offset;
uniform float depth;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    fragColor = vec4(0);

    for (float z = 0.; z <= 1; z += offset) {
        fragColor += texture(textures, vec3(texCoord, z));
    }

    fragColor /= depth;

    fragColor = gammaCorrect(fragColor);
}
