
uniform isampler1D caseToNumPolysData;
uniform isampler1D triangleTableData;
uniform sampler3D worldVolume;
uniform vec3 edgeDim;
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
layout(triangle_strip, max_vertices = 5) out;

#define TRI_POINT(e) mix(coords[e.x], coords[e.y], vec3(abs(vs[e.x] / (vs[e.y] - vs[e.x]))))

void main() {
    vec3 coord = gl_in[0].gl_Position.xyz;

    vec3 coords[8];
    float vs[8];

    coords[0] = coord; vs[0] = sampleVolume(coord); coord.y += edgeDim.y;
    coords[1] = coord; vs[1] = sampleVolume(coord); coord.x += edgeDim.x;
    coords[2] = coord; vs[2] = sampleVolume(coord); coord.y -= edgeDim.y;
    coords[3] = coord; vs[3] = sampleVolume(coord); coord.x -= edgeDim.z; coord.z += edgeDim.z;
    
    coords[4] = coord; vs[4] = sampleVolume(coord); coord.y += edgeDim.y;
    coords[5] = coord; vs[5] = sampleVolume(coord); coord.x += edgeDim.x;
    coords[6] = coord; vs[6] = sampleVolume(coord); coord.y -= edgeDim.y;
    coords[7] = coord; vs[7] = sampleVolume(coord);

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
