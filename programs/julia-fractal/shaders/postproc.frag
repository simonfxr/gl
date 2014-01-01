#include "quasi_constants.h"

uniform sampler2DMS texture0;

in vec2 texCoord;

out vec4 fragColor;

void main() {
    ivec2 index = ivec2(textureSize(texture0) * texCoord);
    
    vec4 sum = vec4(0);
    for (int i = 0; i < NUM_SAMPLES; ++i)
        sum += texelFetch(texture0, index, i);

    fragColor = sum * (1 / float(NUM_SAMPLES));
}
