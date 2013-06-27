#include "glow_pass_constants.h"

uniform sampler2D texture0;
uniform float kernel[KERNEL_SIZE];

in vec2 texCoord;
out vec4 color;

const int N = KERNEL_SIZE;
const float N2 = (N - 1) * 0.5;

void main() {
    vec4 samp = vec4(0);
    vec2 texelSize = vec2(1) / (textureSize(texture0, 0) - vec2(1));
    
    for (int i = 0; i < N; ++i) {
        float x = float(i) - N2;
        samp += kernel[i] * texture(texture0, texCoord + vec2(x, 0) * texelSize);
    }
    
    color = samp;
}
