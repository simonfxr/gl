
#need "point_light.frag"

void
PointLight(vec3 ecLight,
           vec3 attenuation,
           vec4 lightAmbient,
           vec4 lightDiffuse,
           vec4 lightSpecular,
           vec3 ecPosition,
           vec3 ecNormal,
           float shininess,
           inout vec4 ambient,
           inout vec4 diffuse,
           inout vec4 specular);
