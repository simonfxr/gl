#include "gamma.h"

uniform sampler2D textures;
uniform float offset;
uniform float depth;

SL_in vec2 texCoord;

DEF_FRAG_COLOR

void
main()
{

    /* fragColor = vec4(0.); */

    /* for (int i = 0; i < 4; ++i) { */
    /*     fragColor += texture(textures, texCoord, i); */
    /* } */

    /* for (float z = 0.; z <= 1; z += offset) { */
    /*     fragColor += texture(textures, vec3(texCoord, z)); */
    /* } */

    /* fragColor /= depth; */

    FragColor = texture2D(textures, texCoord);
    FragColor = gammaCorrect(FragColor);
}
