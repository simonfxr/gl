#include <cstring>

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

void DynBatch::send(const Attr attrs[]) {
    GL_CHECK(glGenBuffers(nattrs, buffer_names));

    for (uint32 i = 0; i < nattrs; ++i) {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer_names[i]));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, componentSize(attrs[i].type) * attrs[i].size * filled, data[i], GL_STATIC_DRAW));
        delete [] data[i];
        data[i] = 0;
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

} // namespace priv

} // namespace glt
