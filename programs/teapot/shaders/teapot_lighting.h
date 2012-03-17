
#need "teapot_lighting.frag"

vec4 TeapotLighting(vec3 position, vec3 normal, vec3 lightPos,
                    vec4 intensity_diff, vec4 intensity_spec, float shininess);

const bool renderNormal = true;
