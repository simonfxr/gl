#include "gamma.h"

vec4 gammaCorrect(vec4 color) {
    vec4 c = vec4(pow(color.rgb, vec3(1. / gammaCorrection)), color.a);
    return clamp(c, vec4(0), vec4(1));
}
