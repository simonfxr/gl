uniform sampler3D noise;

in vec3 vTexCoord;
out vec4 fColor;

void
main()
{
    //    fColor = vec4(1, 0, 0, 1);
    fColor = vec4(vec3(texture(noise, vTexCoord).r), 1);
}
