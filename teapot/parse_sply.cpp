#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

#include "parse_sply.hpp"

#include "math/vec3.hpp"

using namespace math;

int32 parse_sply(const char *filename, glt::GenBatch<Vertex>& model) {
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
        int nparsed = sscanf(line, "%f %f %f %f %f %f", &p.x, &p.y, &p.z, &n.x, &n.y, &n.z);

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

    while (fgets(line, sizeof line, data) != 0 && faces < nfaces) {
        uint32 n, i, j, k, l;
        int nparsed = sscanf(line, "%u %u %u %u %u", &n, &i, &j, &k, &l);

        if (nparsed != 5 || n != 4) {
            fclose(data);
            return -1;
        }

        model.add(verts.at(i));
        model.add(verts.at(j));
        model.add(verts.at(k));
        model.add(verts.at(l));

        ++faces;
    }

    fclose(data);

    return int32(nfaces) * 4;

//    return faces == nfaces ? int32(nfaces) * 4: -1;
}
