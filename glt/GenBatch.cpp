#include <cstring>
#include <cstdio>

#include "glt/GenBatch.hpp"

namespace glt {

namespace {

byte *resizeArr(byte *in, uint32 size, uint32 newsize) {
    byte *out = new byte[newsize];
    memcpy(out, in, size);
    delete [] in;
    return out;
}

uint32 componentSize(GLenum type) {
    switch (type) {
    case GL_FLOAT: return sizeof (GLfloat);
    case GL_UNSIGNED_BYTE: return sizeof (GLubyte);
    }

    FATAL_ERROR("unknown OpenGL component type");
}

} // namespace anon

namespace priv {

DynBatch::DynBatch(uint32 _nattrs, const Attr attrs[], uint32 initialSize) :
    size(initialSize),
    filled(0),
    buffer_names(new GLuint[_nattrs]),
    data(new byte *[_nattrs]),
    nattrs(_nattrs)
{
    for (uint32 i = 0; i < nattrs; ++i)
        data[i] = new byte[componentSize(attrs[i].type) * attrs[i].size * initialSize];
}

DynBatch::~DynBatch() {
    GL_CHECK(glDeleteBuffers(nattrs, buffer_names));
    for (uint32 i = 0; i < nattrs; ++i)
        delete[] data[i];
    delete[] data;
}

void DynBatch::add(const Attr attrs[], const void *value) {
    
    if (unlikely(filled >= size)) {
        uint32 oldSize = size;
        size *= 2;
        
        for (uint32 i = 0; i < nattrs; ++i) {
            uint32 valSize = componentSize(attrs[i].type) * attrs[i].size;
            data[i] = resizeArr(data[i], valSize * oldSize, valSize * size);
        }
    }

    const byte *bytes = (const byte *) value;

    for (uint32 i = 0; i < nattrs; ++i) {
        uint32 valSize = componentSize(attrs[i].type) * attrs[i].size;
        memcpy(data[i] + valSize * filled, &bytes[(uint64) attrs[i].offset], valSize);
    }

    ++filled;
}

void DynBatch::send(const Attr attrs[], bool del) {
    GL_CHECK(glGenBuffers(nattrs, buffer_names));

    for (uint32 i = 0; i < nattrs; ++i) {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer_names[i]));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, componentSize(attrs[i].type) * attrs[i].size * filled, data[i], GL_STATIC_DRAW));

        if (del) {
            delete [] data[i];
            data[i] = 0;
        }
    }
}

void DynBatch::draw(const Attr attrs[], GLenum primType, bool enabled[]) {

    for (uint32 i = 0; i < nattrs; ++i) {
        if (!enabled[i]) continue;

        GL_CHECK(glEnableVertexAttribArray(i));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer_names[i]));
        GL_CHECK(glVertexAttribPointer(i, attrs[i].size, attrs[i].type, attrs[i].normalized, 0, 0));
    }

    GL_CHECK(glDrawArrays(primType, 0, filled));

    for (uint32 i = 0; i < nattrs; ++i) {
        if (enabled[i])
            GL_CHECK(glDisableVertexAttribArray(i));
    }
}

void DynBatch::at(const Attr attrs[], uint32 i, void *buffer) const {
    ASSERT(i < filled);
    byte *dest = (byte *) buffer;
    for (uint32 k = 0; k < nattrs; ++k) {
        uint32 valSize = componentSize(attrs[k].type) * attrs[k].size;
        memcpy(&dest[(uint64) attrs[k].offset], data[k] + valSize * i, valSize);
    }
}

static const char *HEADER = "GENBATCHMODELFILE\n";
static uint32 VERS_MAJ = 0;
static uint32 VERS_MIN = 1;

bool DynBatch::read(const char *file, const Attr attrs[]) {
    FILE *fd = fopen(file, "rb");
    if (!fd)
        return false;

    bool success = false;
    byte **new_data = 0;
    char line[strlen(HEADER) + 1];
    uint32 num_vertices, pow2;


    if (fread(line, sizeof line - 1, 1, fd) != 1)
        goto ret;

    line[sizeof line - 1] = 0;

    if (strcmp(HEADER, line) != 0) {
        ERROR("not a model file!");
        goto ret;
    }

    uint32 vers_maj, vers_min;

    if (fscanf(fd, "VERSION:%u.%u\n", &vers_maj, &vers_min) != 2)
        goto ret;

    if (vers_maj != VERS_MAJ || vers_min != VERS_MIN) {
        ERROR("versions dont match!");
        goto ret;
    }

    uint32 num_attrs;
    if (fread(&num_attrs, sizeof num_attrs, 1, fd) != 1)
        goto ret;

    if (num_attrs != nattrs)
        goto ret;

    for (uint32 i = 0; i < nattrs; ++i) {
        Attr a;
        if (fread(&a, sizeof a, 1, fd) != 1)
            goto ret;
        if (memcmp(&a, &attrs[i], sizeof a) != 0) {
            ERROR("attributes dont match!");
            goto ret;
        }
    }

    if (fread(&num_vertices, sizeof num_vertices, 1, fd) != 1)
        goto ret;

    pow2 = 4;
    while (pow2 < num_vertices)
        pow2 *= 2;

    new_data = new byte*[nattrs];
    memset(new_data, 0, sizeof new_data[0] * nattrs);

    for (uint32 i = 0; i < nattrs; ++i) {
        uint32 valSize = componentSize(attrs[i].type) * attrs[i].size;
        new_data[i] = new byte[pow2 * valSize];

        if (fread(new_data[i], valSize, num_vertices, fd) != num_vertices)
            goto ret;
    }

    for (uint32 i = 0; i < nattrs; ++i)
        delete[] data[i];
    
    delete[] data;
    data = new_data;
    new_data = 0;

    size = pow2;
    filled = num_vertices;
    success = true;
    
ret:

    if (new_data != 0) {
        for (uint32 i = 0; i < nattrs; ++i)
            delete[] new_data[i];
        delete[] new_data;
    }
    
    fclose(fd);
    return success;
}

bool DynBatch::write(const char *file, const Attr attrs[]) const {
    FILE *fd = fopen(file, "wb");
    if (fd == 0)
        return false;

    bool success = false;

    if (fwrite(HEADER, strlen(HEADER), 1, fd) != 1)
        goto ret;

    if (fprintf(fd, "VERSION:%u.%u\n", VERS_MAJ, VERS_MIN) < 0)
        goto ret;

    if (fwrite(&nattrs, sizeof nattrs, 1, fd) != 1)
        goto ret;

    if (fwrite(attrs, sizeof attrs[0], nattrs, fd) != nattrs)
        goto ret;

    if (fwrite(&filled, sizeof filled, 1, fd) != 1)
        goto ret;

    for (uint32 i = 0; i < nattrs; ++i) {
        uint32 valSize = componentSize(attrs[i].type) * attrs[i].size;
        if (fwrite(data[i], valSize, filled, fd) != filled)
            goto ret;
    }

    success = true;

ret:
    fclose(fd);
    return success;
}

} // namespace priv

} // namespace glt
