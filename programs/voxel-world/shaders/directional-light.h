#need "directional-light.frag"

vec4
DirectionalLight(vec3 position,
                 vec3 normal,
                 vec3 lightDir,
                 vec4 intensity_diff,
                 vec4 intensity_spec,
                 float shininess);
