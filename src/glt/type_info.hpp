#ifndef GLT_TYPE_INFO_HPP
#define GLT_TYPE_INFO_HPP

#include "glt/conf.hpp"

#include "data/ArrayView.hpp"
#include "glt/color.hpp"
#include "math/genmat.hpp"
#include "math/genvec.hpp"
#include "math/real.hpp"
#include "pp/enum.hpp"

#include <array>
#include <cstddef>

#if HU_COMP_GNULIKE_P
#define ti_offsetof(t, fld) __builtin_offsetof(t, fld)
#elif defined(offsetof)
#define ti_offsetof offsetof
#else
#error "NEITHER offsetof nor __builtin_offsetof available"
#endif

namespace glt {

DEF_ENUM_CLASS(GLT_API,
               ScalarType,
               uint8_t,
               I8,
               I16,
               I32,
               U8,
               U16,
               U32,
               F32,
               F64);

template<typename T>
struct scalar_type_mapping;

template<typename T>
struct gl_type_mapping;

#define DEF_SCALAR_TYPE_MAPPING(ty, sc)                                        \
    template<>                                                                 \
    struct scalar_type_mapping<ty>                                             \
    {                                                                          \
        static inline constexpr ScalarType scalar_type = ScalarType::sc;       \
    }

#define DEF_GL_TYPE_MAPPING(ty, gl_ty)                                         \
    template<>                                                                 \
    struct gl_type_mapping<ty>                                                 \
    {                                                                          \
        using type = gl_ty;                                                    \
    }
#define DEF_DIRECT_TYPE_MAPPING(ty, sc)                                        \
    DEF_SCALAR_TYPE_MAPPING(ty, sc);                                           \
    DEF_GL_TYPE_MAPPING(ty, ty)

DEF_DIRECT_TYPE_MAPPING(int8_t, I8);
DEF_DIRECT_TYPE_MAPPING(int16_t, I16);
DEF_DIRECT_TYPE_MAPPING(int32_t, I32);

DEF_DIRECT_TYPE_MAPPING(uint8_t, U8);
DEF_DIRECT_TYPE_MAPPING(uint16_t, U16);
DEF_DIRECT_TYPE_MAPPING(uint32_t, U32);

#ifdef MATH_REAL_FLOAT
DEF_DIRECT_TYPE_MAPPING(float, F32);
DEF_DIRECT_TYPE_MAPPING(double, F64);
#else
DEF_DIRECT_TYPE_MAPPING(float, F32);
DEF_SCALAR_TYPE_MAPPING(double, F64);
DEF_GL_TYPE_MAPPING(math::real, float);
#endif

#undef DEF_DIRECT_TYPE_MAPPING
#undef DEF_GL_TYPE_MAPPING
#undef DEF_SCALAR_TYPE_MAPPING

struct TypeInfo
{
#ifdef HU_COMP_MSVC
    // in 15.8 this leads to an internal compiler error
    bool normalized;
    uint16_t size;
#else
    uint16_t normalized : 1;
    uint16_t size : 15;
#endif
    ScalarType scalar_type;
    uint8_t arity;
};

template<typename T>
struct type_info_mapping
{
    static inline constexpr TypeInfo
      type_info{ false, sizeof(T), scalar_type_mapping<T>::scalar_type, 1 };
};

template<typename T, size_t N>
struct type_info_mapping<math::genvec<T, N>>
{
    static inline constexpr TypeInfo component_info =
      type_info_mapping<T>::type_info;
    static inline constexpr TypeInfo type_info{
        component_info.normalized,
        sizeof(math::genvec<T, N>),
        component_info.scalar_type,
        N *component_info.arity,
    };
};

template<typename T, size_t N>
struct gl_type_mapping<math::genvec<T, N>>
{
    using type = math::genvec<typename gl_type_mapping<T>::type, N>;
};

template<typename T, size_t N>
struct type_info_mapping<math::genmat<T, N>>
{
    static inline constexpr TypeInfo component_info =
      type_info_mapping<T>::type_info;
    static inline constexpr TypeInfo type_info{
        component_info.normalized,
        sizeof(math::genmat<T, N>),
        component_info.scalar_type,
        N *N *component_info.arity,
    };
};

template<typename T, size_t N>
struct gl_type_mapping<math::genmat<T, N>>
{
    using type = math::genmat<typename gl_type_mapping<T>::type, N>;
};

template<>
struct type_info_mapping<color>
{
    static inline constexpr TypeInfo type_info{ true /* normalized */,
                                                sizeof(color),
                                                ScalarType::U8,
                                                4 };
};

template<>
struct gl_type_mapping<color>
{
    using type = color;
};

struct FieldInfo
{
    const char *name;
    TypeInfo type_info;
    uint16_t offset;
    constexpr FieldInfo(const char *nm, TypeInfo ti, uint16_t off)
      : name(nm), type_info(ti), offset(off)
    {}
};

struct StructInfo
{
    const char *name;
    uint16_t size;
    uint16_t align;
    ArrayView<const FieldInfo> fields;
    constexpr StructInfo(const char *nm,
                         uint16_t sz,
                         uint16_t algn,
                         ArrayView<const FieldInfo> fs)
      : name(nm), size(sz), align(algn), fields(fs)
    {}
};

#define TI_DEF_FIELD_INFO0(type, field_name) PP_TOSTR(field_name)
#define TI_DEF_FIELD_INFO(struct_type, type_and_name)                          \
    ::glt::FieldInfo(                                                          \
      PP_DEFER1(TI_DEF_FIELD_INFO0) type_and_name,                             \
      ::glt::type_info_mapping<PP_DEFER1(PP_ARG1) type_and_name>::type_info,   \
      ti_offsetof(struct_type, PP_DEFER1(PP_ARG2) type_and_name))

#define TI_DEF_STRUCT_FIELD0(t, nm) t nm
#define TI_DEF_STRUCT_FIELD(type_and_name)                                     \
    PP_DEFER1(TI_DEF_STRUCT_FIELD0) type_and_name

#define TI_DEF_GL_STRUCT_FIELD0(t, nm)                                         \
    typename ::glt::gl_type_mapping<t>::type nm
#define TI_DEF_GL_STRUCT_FIELD(type_and_name)                                  \
    PP_DEFER1(TI_DEF_GL_STRUCT_FIELD0) type_and_name

#define TI_CONST_1(...) 1
#define TI_COUNT(...) PP_MAP(TI_CONST_1, PP_PLUS, __VA_ARGS__)

#define TI_VAR(x) PP_CAT(PP_CAT(ti_tmp_, x), __LINE__)

#define TI_INIT_GL_FIELD0(ty, fld)                                             \
    fld(static_cast<typename ::glt::gl_type_mapping<ty>::type>(TI_VAR(arg).fld))
#define TI_INIT_GL_FIELD(type_and_field)                                       \
    PP_DEFER1(TI_INIT_GL_FIELD0) type_and_field

#define TI_INIT_FIELD0(ty, fld) fld(TI_VAR(arg).fld)
#define TI_INIT_FIELD(type_and_field) PP_DEFER1(TI_INIT_FIELD0) type_and_field

#define DEF_GL_MAPPED_TYPE(sname, ...)                                         \
    struct sname                                                               \
    {                                                                          \
        PP_MAP(TI_DEF_STRUCT_FIELD, PP_SEMI, __VA_ARGS__);                     \
        struct gl;                                                             \
        sname() = default;                                                     \
        inline sname(const gl &);                                              \
    };                                                                         \
    struct sname::gl                                                           \
    {                                                                          \
        struct struct_info;                                                    \
        PP_MAP(TI_DEF_GL_STRUCT_FIELD, PP_SEMI, __VA_ARGS__);                  \
        gl() = default;                                                        \
        explicit gl(const sname &TI_VAR(arg))                                  \
          : PP_MAP(TI_INIT_GL_FIELD, PP_COMMA, __VA_ARGS__)                    \
        {}                                                                     \
    };                                                                         \
    struct sname::gl::struct_info                                              \
    {                                                                          \
        static constexpr ::glt::FieldInfo _fields[] = { PP_MAP_WITH_ARG(       \
          TI_DEF_FIELD_INFO,                                                   \
          PP_COMMA,                                                            \
          sname::gl,                                                           \
          __VA_ARGS__) };                                                      \
        static constexpr ::glt::StructInfo info =                              \
          ::glt::StructInfo(PP_TOSTR(sname),                                   \
                            sizeof(sname::gl),                                 \
                            alignof(sname::gl),                                \
                            ::ArrayView<const ::glt::FieldInfo>(               \
                              _fields,                                         \
                              sizeof _fields / sizeof(::glt::FieldInfo)));     \
    };                                                                         \
    sname::sname(const sname::gl &TI_VAR(arg))                                 \
      : PP_MAP(TI_INIT_FIELD, PP_COMMA, __VA_ARGS__)                           \
    {}

GLT_API unsigned toGLScalarType(ScalarType);

} // namespace glt

#endif
