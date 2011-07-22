#ifndef VERTEX_DESCRIPTION_HPP
#define VERTEX_DESCRIPTION_HPP

#include "error/error.hpp"

template <typename T>
struct VertexTraits {
    // static const VertexDesc<T>& description();
};

namespace glt {

struct Any;
struct AttrBase;
template <typename T> struct Attr;
template <typename T> struct VertexDesc;
typedef VertexDesc<Any> VertexDescBase;

struct AttrBase {
    uint32 offset;
    uint32 alignment;
    GLenum component_type;
    uint32 ncomponents;
    bool normalized;
    const char * name;

    AttrBase();
    AttrBase(size_t off, uint32 align, GLenum ty, uint32 ncomp, bool norm = false) :
        offset(uint32(off)),
        alignment(align),
        component_type(ty),
        ncomponents(ncomp),
        normalized(norm),
        name(0)
        {}        
};

template <typename T>
struct Attr : public AttrBase {
    Attr(size_t off, uint32 align, GLenum ty, uint32 ncomp, bool norm = false) :
        AttrBase(off, align, ty, ncomp, norm) {}
    Attr(const AttrBase& base) :
        AttrBase(base) {}
};

template <typename T>
struct VertexDesc {
    uint32 sizeof_vertex;
    uint32 alignment;
    uint32 nattributes;
    const Attr<T> *attributes;

    VertexDesc() :
        sizeof_vertex(0),
        alignment(0),
        nattributes(0),
        attributes(0) {}

    uint32 index(size_t off) const {
        for (uint32 i = 0; i < nattributes; ++i) {
            if (off == attributes[i].offset)
                return i;
        }
        
        FATAL_ERR("invalid attribute offset");
    }

    const VertexDescBase generic() const {
        VertexDescBase desc;
        desc.sizeof_vertex = sizeof_vertex;
        desc.alignment = alignment;
        desc.nattributes = nattributes;
        desc.attributes = (const Attr<Any> *) attributes;
        return desc;
    }
};

} // namespace glt

#endif
