
// packed each of the normalized rgb components in 7 bit and the alpha
// in 3 bit
vec4 encodeColor(vec4 col) {
//    col.rgb = col.gbr;
//    col.a = 1;
    ivec4 rgba = ivec4(clamp(col, vec4(0), vec4(1)) * vec4(vec3(127), 7));
    uint b1 = (rgba.r << 1) | (rgba.g >> 6);
    uint b2 = ((rgba.g << 2) & 255) | (rgba.b >> 5);
    uint b3 = ((rgba.b << 3) & 255) | rgba.a;
    col = vec4(vec3(ivec3(b1, b2, b3)) / vec3(255), 1);
    return col;
}

vec4 decodeColor(vec4 col) {
    ivec3 packed = ivec3(clamp(col.rgb, vec3(0), vec3(1)) * vec3(255));
    ivec4 rgba;
    rgba.r = packed.r >> 1;
    rgba.g = ((packed.r & 1) << 6) | (packed.g >> 2);
    rgba.b = ((packed.g & 3) << 5) | (packed.b >> 3);
    rgba.a = packed.b & 7;
    col = vec4(vec4(rgba) / vec4(vec3(127), 7));
//    col.rgb = col.brg;
//    col.a = 1;
    return col;
}
