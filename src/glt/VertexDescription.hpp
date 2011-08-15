#ifndef VERTEX_DESCRIPTION_HPP
#define VERTEX_DESCRIPTION_HPP

#include "err/err.hpp"

template <typename T>
struct VertexTraits {
    // static const VertexDesc<T>& description();
};

namespace glt {

using namespace defs;

struct Any;
struct AttrBase;
template <typename T> struct Attr;
template <typename T> struct VertexDesc;
typedef VertexDesc<Any> VertexDescBase;

struct AttrBase {
    defs::index offset;
    defs::size alignment;
    GLenum component_type;
    defs::size ncomponents;
    bool normalized;
    const char * name;

    AttrBase();
    AttrBase(defs::index off, defs::size align, GLenum ty, defs::size ncomp, bool norm = false) :
        offset(off),
        alignment(align),
        component_type(ty),
        ncomponents(ncomp),
        normalized(norm),
        name(0)
        {}        
};

template <typename T>
struct Attr : public AttrBase {
    Attr(defs::index off, defs::size align, GLenum ty, defs::size ncomp, bool norm = false) :
        AttrBase(off, align, ty, ncomp, norm) {}
    Attr(const AttrBase& base) :
        AttrBase(base) {}
};

template <typename T>
struct VertexDesc {
    size sizeof_vertex;
    size alignment;
    size nattributes;
    const Attr<T> *attributes;

    VertexDesc() :
        sizeof_vertex(0),
        alignment(0),
        nattributes(0),
        attributes(0) {}

    defs::index index(defs::index off) const {
        for (defs::index i = 0; i < nattributes; ++i) {
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
        desc.attributes = reinterpret_cast<const Attr<Any> *>(attributes);
        return desc;
    }
};

} // namespace glt

#endif
