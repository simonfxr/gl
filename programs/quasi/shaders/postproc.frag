#include "quasi_constants.h"

uniform sampler2DMS texture0;

SL_in vec2 texCoord;
DEF_FRAG_COLOR

void
main()
{
    ivec2 index = ivec2(textureSize(texture0) * texCoord);

    vec4 sum = vec4(0);
    for (int i = 0; i < 4; ++i)
        sum += texelFetch(texture0, index, i);

    FragColor = sum * (1. / float(NUM_SAMPLES));
}
