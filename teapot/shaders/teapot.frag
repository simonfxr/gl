

#include "gamma.h"
#include "point_light2.h"

uniform vec3 ecLight;
uniform vec4 surfaceColor;
uniform vec4 materialProperties;

in vec3 ecNormal;
in vec3 ecPosition;

out vec4 color;

void main() {
    float ambientContribution = materialProperties.x;
    float diffuseContribution = materialProperties.y;
    float specularContribution = materialProperties.z;
    float shininess = materialProperties.w;

    vec3 radiance = vec3(PointLight(ecPosition, normalize(ecNormal), ecLight,
                                    vec4(diffuseContribution), vec4(specularContribution), shininess));
    
    vec3 shaded_rgb = (radiance + vec3(ambientContribution)) * surfaceColor.rgb;
    color = gammaCorrect(vec4(min(vec3(1), shaded_rgb), surfaceColor.a));
}
