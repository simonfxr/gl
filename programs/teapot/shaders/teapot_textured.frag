

// #include "gamma.h"
// #include "point_light2.h"
#include "teapot_lighting.h"

uniform vec3 ecLight;
uniform vec4 surfaceColor;
uniform vec4 materialProperties;
uniform sampler2D texData;
uniform float glow;

in vec3 ecNormal;
in vec3 ecPosition;
in vec2 fragTexCoord;

out vec4 color;

void main() {
    if (renderNormal) {
        color = vec4(normalize(ecNormal), glow);
    } else {
        float ambientContribution = materialProperties.x;
        float diffuseContribution = materialProperties.y;
        float specularContribution = materialProperties.z;
        float shininess = materialProperties.w;
        
        vec3 radiance = vec3(TeapotLighting(ecPosition, normalize(ecNormal), ecLight,
                                            vec4(diffuseContribution), vec4(specularContribution), shininess));
        
        vec4 baseColor = texture(texData, fragTexCoord);
        vec3 shaded_rgb = (radiance + vec3(ambientContribution)) * baseColor.rgb;
        color = vec4(min(vec3(1), shaded_rgb), glow);
//    color = gammaCorrect(color);
    }
}
