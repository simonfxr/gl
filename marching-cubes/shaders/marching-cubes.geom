
uniform isampler1D caseToNumPolysData;
uniform isampler1D triangleTableData;
uniform sampler3D worldVolume;
uniform vec3 edgeDim;
uniform vec3 texEdgeDim;
uniform float isoLevel;
uniform mat4 vpMatrix;

int caseToNumPolys(int cas) {
    return texture(caseToNumPolysData, cas).x;
}

ivec3 triangleTable(int cas, int num) {
    return texture(triangleTableData, cas + 5 * num).xyz;
}

float sampleVolume(vec3 coord) {
    return texture(worldVolume, coord).r;
}

const ivec2 edge_to_verts[12] = ivec2[12](
    ivec2(0, 1), ivec2(1, 2), ivec2(2, 3),
    ivec2(3, 0), ivec2(4, 5), ivec2(5, 6),
    ivec2(6, 7), ivec2(7, 4), ivec2(0, 4),
    ivec2(1, 5), ivec2(2, 6), ivec2(3, 7)
);

ivec2 edgeToVertices(int e) {
    return edge_to_verts[e];
}

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

in vec3 vTexCoord[1];               

#define TRI_POINT(e) mix(wcs[e.x], wcs[e.y], vec3(abs(vs[e.x] / (vs[e.y] - vs[e.x]))))

void main() {
    vec3 wc = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 tc = vTexCoord[0];

    /* vec3 wc0 = coord; */
    /* vec3 wc1 = coord + vec3(edgeDim.x, 0, 0); */
    /* vec3 wc2 = coord + vec3(0, edgeDim.y, 0); */

    /* gl_Position = vpMatrix * vec4(wc0, 1); EmitVertex(); */
    /* gl_Position = vpMatrix * vec4(wc2, 1); EmitVertex(); */
    /* gl_Position = vpMatrix * vec4(wc1, 1); EmitVertex(); */
    /* EndPrimitive(); */

    vec3 wcs[8];
    float vs[8];

    wcs[0] = wc; vs[0] = sampleVolume(tc); wc.y += edgeDim.y; tc.y += texEdgeDim.y;
    wcs[1] = wc; vs[1] = sampleVolume(tc); wc.x += edgeDim.x; tc.x += texEdgeDim.x;
    wcs[2] = wc; vs[2] = sampleVolume(tc); wc.y -= edgeDim.y; tc.y -= texEdgeDim.y;
    wcs[3] = wc; vs[3] = sampleVolume(tc); wc.x -= edgeDim.x; wc.z += edgeDim.z;
    tc.x -= texEdgeDim.x; tc.z += texEdgeDim.z;
    
    wcs[4] = wc; vs[4] = sampleVolume(tc); wc.y += edgeDim.y; tc.y += texEdgeDim.y;
    wcs[5] = wc; vs[5] = sampleVolume(tc); wc.x += edgeDim.x; tc.x += texEdgeDim.x;
    wcs[6] = wc; vs[6] = sampleVolume(tc); wc.y -= edgeDim.y; tc.y -= texEdgeDim.y;
    wcs[7] = wc; vs[7] = sampleVolume(tc);

    int cas = 0;
    cas += int(vs[0] < isoLevel) << 0;
    cas += int(vs[1] < isoLevel) << 1;
    cas += int(vs[2] < isoLevel) << 2;
    cas += int(vs[3] < isoLevel) << 3;
    cas += int(vs[4] < isoLevel) << 4;
    cas += int(vs[5] < isoLevel) << 5;
    cas += int(vs[6] < isoLevel) << 6;
    cas += int(vs[7] < isoLevel) << 7;

    if (cas == 0 || cas == 255)
        return;

    int ntri = caseToNumPolys(cas);
    for (int i = 0; i < ntri; ++i) {
        ivec3 tri = triangleTable(cas, i);
        ivec2 edge;
        vec3 wc;

        edge = edgeToVertices(tri.x); wc = TRI_POINT(edge);
        gl_Position = vpMatrix * vec4(wc, 1);
        EmitVertex();

        edge = edgeToVertices(tri.y); wc = TRI_POINT(edge);
        gl_Position = vpMatrix * vec4(wc, 1);
        EmitVertex();

        edge = edgeToVertices(tri.z); wc = TRI_POINT(edge);
        gl_Position = vpMatrix * vec4(wc, 1);
        EmitVertex();

        EndPrimitive();
    }
}
