#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

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

__kernel void computeHistogramLevel(
    __read_only image3d_t source,
    __write_only image3d_t dest)
{
    int4 wpos = { get_global_id(0), get_global_id(1), get_global_id(2), 0 };
    int4 rpos = wpos * 2;

    int count = 0;
    count += read_imagei(source, sampler, rpos + cube_offsets[0]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[1]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[2]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[3]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[4]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[5]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[6]).x;
    count += read_imagei(source, sampler, rpos + cube_offsets[7]).x;

    write_imagei(dest, wpos, count);
}

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

int4 scanHistogramLevel(int index, __read_only image3d_t histo, int4 pos) {

    int8 neighbors = {
        read_imagei(histo, sampler, pos + cube_offsets[0]).x,
        read_imagei(histo, sampler, pos + cube_offsets[1]).x,
        read_imagei(histo, sampler, pos + cube_offsets[2]).x,
        read_imagei(histo, sampler, pos + cube_offsets[3]).x,
        read_imagei(histo, sampler, pos + cube_offsets[4]).x,
        read_imagei(histo, sampler, pos + cube_offsets[5]).x,
        read_imagei(histo, sampler, pos + cube_offsets[6]).x,
        read_imagei(histo, sampler, pos + cube_offsets[7]).x
    };
    
    int acc = pos.s3;
    int8 cmp;

    acc += neighbors.s0;
    cmp.s0 = acc <= index;
    acc += neighbors.s1;
    cmp.s1 = acc <= index;
    acc += neighbors.s2;
    cmp.s2 = acc <= index;
    acc += neighbors.s3;
    cmp.s3 = acc <= index;
    acc += neighbors.s4;
    cmp.s4 = acc <= index;
    acc += neighbors.s5;
    cmp.s5 = acc <= index;
    acc += neighbors.s6;
    cmp.s6 = acc <= index;
    cmp.s7 = 0;

    pos += cube_offsets[cmp.s0 + cmp.s1 + cmp.s2 + cmp.s3 +
                        cmp.s4 + cmp.s5 + cmp.s6 + cmp.s7];

    pos.s0 *= 2;
    pos.s1 *= 2;
    pos.s2 *= 2;
    
    neighbors *= cmp;
    pos.s3 +=
        neighbors.s0 + neighbors.s1 + neighbors.s2 + neighbors.s3 +
        neighbors.s4 + neighbors.s5 + neighbors.s6 + neighbors.s7;
    
    return pos;
}

float3 calculateGradient(__read_only image3d_t h0, int4 p) {
    int dx = read_imagei(h0, sampler, p + (int4) { 1, 0, 0, 0 }).s2 -
        read_imagei(h0, sampler, (int4) { -1, 0, 0, 0 }).s2;
    int dy = read_imagei(h0, sampler, p + (int4) { 0, 1, 0, 0 }).s2 -
        read_imagei(h0, sampler, (int4) { 0, -1, 0, 0 }).s2;
    int dz = read_imagei(h0, sampler, p + (int4) { 0, 0, 1, 0 }).s2 -
        read_imagei(h0, sampler, (int4) { 0, 0, -1, 0 }).s2;

    // should be halfed, but doesnt matter, will be normalized anyway
    return convert_float3((int3) { dx, dy, dz });
}

float3 normalize3(float3 a) {
    float il = rsqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    return a * il;
}

__kernel void histogramTraversal(
    __read_only image3d_t h0,
    __read_only image3d_t h1,
    __read_only image3d_t h2,
       __read_only image3d_t h3,
    __read_only image3d_t h4,
    __read_only image3d_t h5,
    #if N > 64
    __read_only image3d_t h6,
    #endif
    #if N > 128
    __read_only image3d_t h7,
    #endif
    #if N > 256
    __read_only image3d_t h8,
    #endif
    __global float *vbo,
    __private int isolevel,
    __private int num_tris)
{

    int index = get_global_id(0);
    if (index >= num_tris)
        index = 0;

    int4 cube_position = (int4) 0;

    #if N > 256
    cube_position = scanHistogramLevel(index, h8, cube_position);
    #endif
    #if N > 128
    cube_position = scanHistogramLevel(index, h7, cube_position);
    #endif
    #if N > 64
    cube_position = scanHistogramLevel(index, h6, cube_position);
    #endif
    cube_position = scanHistogramLevel(index, h5, cube_position);
    cube_position = scanHistogramLevel(index, h4, cube_position);
    cube_position = scanHistogramLevel(index, h3, cube_position);
    cube_position = scanHistogramLevel(index, h2, cube_position);
    cube_position = scanHistogramLevel(index, h1, cube_position);
    cube_position = scanHistogramLevel(index, h0, cube_position);

    cube_position.s0 /= 2;
    cube_position.s1 /= 2;
    cube_position.s2 /= 2;

    const int4 cube_data = read_imagei(h0, sampler, cube_position);

    int vertex_index = index * 6;
    for (int i = (index - cube_position.s3) * 3; i < (index - cube_position.s3 + 1) * 3; ++i) {
        const uchar edge = triangle_edge_table[cube_data.s1 * 16 + i];
        const int4 p0 = (int4)(cube_position.x + offsets3[edge * 6 + 0],
                               cube_position.y + offsets3[edge * 6 + 1],
                               cube_position.z + offsets3[edge * 6 + 2], 0);
        
        const int4 p1 = (int4)(cube_position.x + offsets3[edge * 6 + 3],
                               cube_position.y + offsets3[edge * 6 + 4],
                               cube_position.z + offsets3[edge * 6 + 5], 0);

        float3 n0 = normalize3(- calculateGradient(h0, p0));
        float3 n1 = normalize3(- calculateGradient(h0, p1));

        int v0 = read_imagei(h0, sampler, p0).s2;
        int v1 = read_imagei(h0, sampler, p1).s2;
        const float t = native_divide((float)(isolevel - v0), (float)(v1 - v0));

        float3 pos = mix(convert_float3(p0.xyz), convert_float3(p1.xyz), t);
        float3 normal = mix(n0, n1, t);

        vstore3(pos, vertex_index * 2, vbo);
        vstore3(normal, vertex_index * 2 + 1, vbo);
        ++vertex_index;
    }

}
