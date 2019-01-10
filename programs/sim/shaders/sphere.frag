

#include "gamma.h"
#include "sim_shading.h"

uniform vec3 ecLight;
uniform vec4 color;
uniform float shininess;
uniform vec3 ecSpotDirection;
uniform float spotAngle;

vec4
shade(vec3 ecLight,
      vec3 ecPosition,
      vec3 ecNormal,
      float shininess,
      vec4 color);

SL_in vec3 ecPosition;
SL_in vec3 ecNormal;

SL_out vec4 fragColor;

void
main()
{
    fragColor =
      shade(ecLight, ecPosition, normalize(ecNormal), shininess, color);
    fragColor = gammaCorrect(fragColor);
}
