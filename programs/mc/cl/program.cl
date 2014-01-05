
constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__constant uchar triangle_count_table[256] = {
#include "cl/triangle_count_table.h"
};

__constant char triangle_edge_table[4096] = {
#include "cl/triangle_edge_table.h"
};

__constant char offsets3[72] = {
    // 0
    0,0,0,
    1,0,0,
    // 1
    1,0,0,
    1,0,1,
    // 2
    1,0,1,
    0,0,1,
    // 3
    0,0,1,
    0,0,0,
    // 4
    0,1,0,
    1,1,0,
    // 5
    1,1,0,
    1,1,1,
    // 6
    1,1,1,
    0,1,1,
    // 7
    0,1,1,
    0,1,0,
    // 8
    0,0,0,
    0,1,0,
    // 9
    1,0,0,
    1,1,0,
    // 10
    1,0,1,
    1,1,1,
    // 11
    0,0,1,
    0,1,1
};

__constant int4 cube_offsets[8] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 1, 0},
}; 

__kernel void computeHistogramBaseLevel(
    __write_only image3d_t histo_base,
    __read_only image3d_t data,
    __private int isolevel)
{
    int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };

    uchar value = read_imagei(data, sampler, pos).x;
    uchar cube_index = 0;

    cube_index |= value > isolevel;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[1]).x > isolevel) << 1;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[3]).x > isolevel) << 2;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[2]).x > isolevel) << 3;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[4]).x > isolevel) << 4;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[5]).x > isolevel) << 5;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[7]).x > isolevel) << 6;
    cube_index |= (read_imagei(data, sampler, pos + cube_offsets[6]).x > isolevel) << 7;

    write_imageui(histo_base, pos, (uint4) { triangle_count_table[cube_index], cube_index, value, 1 });
}

typedef float16 mat4;
typedef float4 vec4;
typedef float4 vec3;

vec4 multM4V4(mat4 m, vec4 v) {
    float8 a = m.lo;
    float8 b = m.hi;
    
    float4 m1 = a.lo;
    float4 m2 = a.hi;
    float4 m3 = b.lo;
    float4 m4 = b.hi;

    vec4 c = (float4) 0;
    c += v.s0 * m1;
    c += v.s1 * m2;
    c += v.s2 * m3;
    c += v.s3 * m4;

    return c;
}

float dot3(vec3 a, vec3 b) {
    a *= b;
    return a.s0 + a.s1 + a.s2;
}

__kernel void generateSphereVolume(__write_only image3d_t data,
                                   __private mat4 transform,
                                   __private vec3 center,
                                   __private float radius)
{
    int4 pos = { get_global_id(0), get_global_id(1), get_global_id(2), 1 };
    vec4 wc = multM4V4(transform, convert_float4(pos));

    vec3 dist = wc - center;
    float value = dot3(dist, dist) / (radius * radius)  - 1;

    value *= 128;
    value += 128;

    if (value < 0)
        value = 0; // shouldnt happen
    else if (value > 255)
        value = 255;

    pos.s3 = 0;
    write_imagei(data, pos, (uchar) (int) value);
}
