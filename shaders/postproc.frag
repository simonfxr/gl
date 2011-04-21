#version 330

uniform sampler3D textures;
uniform float depth;

in vec2 texCoord;
out vec4 fragColor;

void main() {
    fragColor = vec4(0);
    float step = depth <= 1 ? 2 : 1 / (depth - 1);
    for (float z = 0.; z <= 1; z += step) {
        fragColor += texture(textures, vec3(texCoord, z));
    }

    fragColor /= depth;
}
