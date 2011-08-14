#ifndef GENBATCH_HPP
#define GENBATCH_HPP

#include "defs.h"
#include "opengl.h"
#include "err/err.hpp"

namespace glt {

struct Attr {
    GLint size;
    GLenum type;
    GLboolean normalized;
    const GLvoid *offset;
};

template <typename T>
struct Attrs {  
    uint32 length;
    const Attr *attrs;

    Attrs(uint32 _length, const Attr _attrs[])
        : length(_length), attrs(_attrs) {}

    GLuint index(uint64 offset) const;
};

template <typename T>
GLuint Attrs<T>::index(uint64 offset) const {
    const GLvoid *off = (const GLvoid *) offset;
    for (uint32 i = 0; i < length; ++i)
        if ((const GLvoid *) off == attrs[i].offset)
            return i;
    FATAL_ERR("unknown offset in attribute list");
}

namespace {

namespace attr {

Attr LOCAL vec2(uint64 offset) {
    Attr a;
    a.size = 2; a.type = GL_FLOAT; a.normalized = GL_FALSE;
    a.offset = (const GLvoid *) offset;
    return a;
}

Attr LOCAL vec3(uint64 offset) {
    Attr a;
    a.size = 3; a.type = GL_FLOAT; a.normalized = GL_FALSE;
    a.offset = (const GLvoid *) offset;
    return a;
}

Attr LOCAL vec4(uint64 offset) {
    Attr a;
    a.size = 4; a.type = GL_FLOAT; a.normalized = GL_FALSE;
    a.offset = (const GLvoid *) offset;
    return a;
}

Attr LOCAL color(uint64 offset) {
    Attr a;
    a.size = 4; a.type = GL_UNSIGNED_BYTE; a.normalized = GL_TRUE;
    a.offset = (const GLvoid *) offset;
    return a;
}

static const Attr null = {
    0, GL_FLOAT, GL_FALSE, NULL
};

} // namespace attr

} // namespace anon

namespace priv {

struct DynBatch {
    uint32 size;
    uint32 filled;

    GLuint *buffer_names;
    byte **data;

    uint32 nattrs;
    
    DynBatch();
    ~DynBatch();

    void init(uint32 nattrs, const Attr attrs[], uint32 initialSize);
    void add(const Attr attrs[], const void *value);
    void send(const Attr attrs[], bool del_after_freeze);
    void draw(const Attr attrs[], GLenum primType, bool enabled[], uint32 num_instances);
    void at(const Attr attrs[], uint32 i, void *buffer) const;
    bool read(const char *file, const Attr attrs[]);
    bool write(const char *file, const Attr attrs[]) const;
};

} // namespace priv

template <typename T>
struct GenBatch {
private:

    GLenum _primType;
    const Attrs<T>* attrs;
    bool frozen;
    bool del_after_freeze;
    priv::DynBatch batch;
    bool *enabledAttrs;
    
    GenBatch(const GenBatch& _);
    GenBatch& operator =(const GenBatch& _);

public:

    GenBatch(const Attrs<T>& _attrs) :
        _primType(GL_TRIANGLES),
        attrs(&_attrs),
        frozen(false),
        del_after_freeze(true),
        enabledAttrs(new bool[_attrs.length])
    {
        batch.init(_attrs.length, _attrs.attrs, 4);
        for (uint32 i = 0; i < attrs->length; ++i)
            enabledAttrs[i] = true;
    }

    GenBatch() :
        _primType(GL_TRIANGLES),
        attrs(0),
        frozen(false),
        del_after_freeze(true),
        enabledAttrs(0)
    {}

    void init(const Attrs<T>& _attrs) {
        ASSERT_MSG(attrs == 0, "already initialized!");
        attrs = &_attrs;
        enabledAttrs = new bool[_attrs.length];
        for (uint32 i = 0; i < attrs->length; ++i)
            enabledAttrs[i] = true;

        batch.init(_attrs.length, _attrs.attrs, 4);
    }


    ~GenBatch() {
        delete [] enabledAttrs;
    }

    const Attrs<T>& getAttrs() { return attrs; }

    void add(const T& v) {
        ASSERT_MSG(!frozen, "cannot add to frozen batch");
        batch.add(attrs->attrs, static_cast<const void *>(&v));
    }

    void freeze() {
        ASSERT_MSG(!frozen, "batch already frozen");
        frozen = true;
        batch.send(attrs->attrs, del_after_freeze);
    }

    void deleteAfterFreeze(bool enable) {
        ASSERT_MSG(!frozen, "already frozen");
        del_after_freeze = enable;
    }

    void draw() {
        ASSERT_MSG(frozen, "cannot draw batch while building it");
        batch.draw(attrs->attrs, _primType, enabledAttrs, 1);
    }

    void drawInstanced(uint32 n) {
        ASSERT_MSG(frozen, "cannot draw batch while building it");
        batch.draw(attrs->attrs, _primType, enabledAttrs, n);
    }

    void enableAttrib(GLuint position, bool enable = true) {
        enabledAttrs[position] = enable;
    }

    void primType(GLenum primType) {
        _primType = primType;
    }
    
    GLenum primType() {
        return _primType;
    }

    uint32 size() {
        return batch.filled;
    }

    T at(uint32 i) const {
        ASSERT_MSG(!frozen || !del_after_freeze, "data already deleted");
        T val;
        batch.at(attrs->attrs, i, static_cast<void *>(&val));
        return val;
    }

    bool read(const char *filename) {
        return batch.read(filename, attrs->attrs);
    }

    bool write(const char *filename) const {
        return batch.write(filename, attrs->attrs);
    }
};

} // namespace glt
#endif
