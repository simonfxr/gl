
void
SpotLight(vec3 ecLight,
          vec3 attenuation,
          vec3 spotDirection,
          float spotCosCutoff,
          float spotExponent,
          vec4 lightAmbient,
          vec4 lightDiffuse,
          vec4 lightSpecular,
          vec3 ecPosition,
          vec3 ecNormal,
          vec3 shininess,
          inout vec4 ambient,
          inout vec4 diffuse,
          inout vec4 specular);
