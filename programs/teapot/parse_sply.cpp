#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

#include "parse_sply.hpp"
#include "math/vec3.hpp"

using namespace math;
using namespace defs;

int32 parse_sply(const char *filename, CubeMesh& model) {
    FILE *data = fopen(filename, "rb");
    if (!data)
        return -1;

    char line[512];

    std::vector<Vertex> verts;
    uint32 nverts = 0;
    uint32 nfaces = 0;

    if (fscanf(data, "%u\n%u\n", &nverts, &nfaces) != 2)
        return -1;

    while (fgets(line, sizeof line, data) != 0 && verts.size() < nverts) {
        point3_t p;
        direction3_t n;
        int nparsed = sscanf(line, "%f %f %f %f %f %f", &p[0], &p[1], &p[2], &n[0], &n[1], &n[2]);

        if (nparsed != 6) {
            fclose(data);
            return -1;
        }

        Vertex v; v.position = p; v.normal = normalize(n);
        verts.push_back(v);
    }

    if (verts.size() != nverts) {
        fclose(data);
        return -1;
    }

    uint32 faces = 0;

#ifdef MESH_MESH

    for (uint32 i = 0; i < verts.size(); ++i)
        model.addVertex(verts[i]);

#endif

    while (fgets(line, sizeof line, data) != 0 && faces < nfaces) {
        uint32 n, i, j, k, l;
        int nparsed = sscanf(line, "%u %u %u %u %u", &n, &i, &j, &k, &l);

        if (nparsed != 5 || n != 4) {
            fclose(data);
            return -1;
        }

#ifdef MESH_MESH
        
        model.addElement(i);
        model.addElement(j);
        model.addElement(k);
        model.addElement(k);
        model.addElement(l);
        model.addElement(i);
        
#else
        
        ADD_VERTEX(model, verts.at(i));
        ADD_VERTEX(model, verts.at(j));
        ADD_VERTEX(model, verts.at(k));
        ADD_VERTEX(model, verts.at(l));

#endif

        ++faces;
    }

    fclose(data);

    return int32(nfaces) * 4;

//    return faces == nfaces ? int32(nfaces) * 4: -1;
}
