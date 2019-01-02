#include <cstdio>
#include <cstring>
#include <vector>

#include "math/vec3.hpp"
#include "parse_sply.hpp"

using namespace math;

struct Vertex
{
    math::point3_t position;
    math::direction3_t normal;
};

int32_t
parse_sply(const char *filename, CubeMesh &model)
{
    FILE *data = fopen(filename, "rb");
    if (!data)
        return -1;

    char line[512];

    std::vector<Vertex> verts;
    uint32_t nverts = 0;
    uint32_t nfaces = 0;

    if (fscanf(data, "%u\n%u\n", &nverts, &nfaces) != 2)
        return -1;

    while (fgets(line, sizeof line, data) != nullptr && verts.size() < nverts) {
        point3_t p;
        direction3_t n;
        int nparsed = sscanf(line,
                             "%" R_FMT " %" R_FMT " %" R_FMT " %" R_FMT
                             " %" R_FMT " %" R_FMT,
                             &p[0],
                             &p[1],
                             &p[2],
                             &n[0],
                             &n[1],
                             &n[2]);

        if (nparsed != 6) {
            fclose(data);
            return -1;
        }

        Vertex v{};
        v.position = p;
        v.normal = normalize(n);
        verts.push_back(v);
    }

    if (verts.size() != nverts) {
        fclose(data);
        return -1;
    }

    uint32_t faces = 0;

    for (const auto &vert : verts)
        model.addVertex(vert);

    while (fgets(line, sizeof line, data) != nullptr && faces < nfaces) {
        uint32_t n, i, j, k, l;
        int nparsed = sscanf(line, "%u %u %u %u %u", &n, &i, &j, &k, &l);

        if (nparsed != 5 || n != 4) {
            fclose(data);
            return -1;
        }

#ifdef MESH_CUBEMESH

        model.addElement(i);
        model.addElement(j);
        model.addElement(k);
        model.addElement(i);
        model.addElement(k);
        model.addElement(l);

#else

        model.addElement(i);
        model.addElement(j);
        model.addElement(k);
        model.addElement(l);

#endif

        ++faces;
    }

    fclose(data);

    return int32_t(nfaces) * 4;

    //    return faces == nfaces ? int32(nfaces) * 4: -1;
}
