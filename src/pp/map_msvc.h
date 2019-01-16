#ifndef PP_MAP_MSVC_H
#define PP_MAP_MSVC_H

#if defined(_MSVC_TRADITIONAL) && !_MSVC_TRADITIONAL
#error "do not include this file directly, use pp/map.h"
#endif

/**
 * We have to work around MSVC's non conforming preprocessor deficiencies...
 * code is adapted from:
 * https://stackoverflow.com/a/29474124
 */

#define PP_ARG_64(_0,                                                          \
                  _1,                                                          \
                  _2,                                                          \
                  _3,                                                          \
                  _4,                                                          \
                  _5,                                                          \
                  _6,                                                          \
                  _7,                                                          \
                  _8,                                                          \
                  _9,                                                          \
                  _10,                                                         \
                  _11,                                                         \
                  _12,                                                         \
                  _13,                                                         \
                  _14,                                                         \
                  _15,                                                         \
                  _16,                                                         \
                  _17,                                                         \
                  _18,                                                         \
                  _19,                                                         \
                  _20,                                                         \
                  _21,                                                         \
                  _22,                                                         \
                  _23,                                                         \
                  _24,                                                         \
                  _25,                                                         \
                  _26,                                                         \
                  _27,                                                         \
                  _28,                                                         \
                  _29,                                                         \
                  _30,                                                         \
                  _31,                                                         \
                  _32,                                                         \
                  _33,                                                         \
                  _34,                                                         \
                  _35,                                                         \
                  _36,                                                         \
                  _37,                                                         \
                  _38,                                                         \
                  _39,                                                         \
                  _40,                                                         \
                  _41,                                                         \
                  _42,                                                         \
                  _43,                                                         \
                  _44,                                                         \
                  _45,                                                         \
                  _46,                                                         \
                  _47,                                                         \
                  _48,                                                         \
                  _49,                                                         \
                  _50,                                                         \
                  _51,                                                         \
                  _52,                                                         \
                  _53,                                                         \
                  _54,                                                         \
                  _55,                                                         \
                  _56,                                                         \
                  _57,                                                         \
                  _58,                                                         \
                  _59,                                                         \
                  _60,                                                         \
                  _61,                                                         \
                  _62,                                                         \
                  _63,                                                         \
                  ...)                                                         \
    _63

// expand expressions

// "return" 2 if there are args, otherwise return 0
// for PP_MAP it's ok that ignore first arg and no case with only
// one arg IMPORTANT! must call as PP_MAP_SWITCH(0, __VA_ARGS__)
// for detection 0/1 arg case
#define PP_MAP_SWITCH(...)                                                     \
    PP_PASS(PP_ARG_64(__VA_ARGS__,                                             \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      3,                                                       \
                      2,                                                       \
                      1,                                                       \
                      0,                                                       \
                      0,                                                       \
                      0))

// macro for recursion
#define PP_MAP_A(...)                                                          \
    PP_CAT(PP_MAP_NEXT_, PP_MAP_SWITCH(0, __VA_ARGS__))(PP_MAP_B, __VA_ARGS__)

#define PP_MAP_B(...)                                                          \
    PP_CAT(PP_MAP_NEXT_, PP_MAP_SWITCH(0, __VA_ARGS__))(PP_MAP_A, __VA_ARGS__)

#define PP_MAP_INVOKE(f, sep, x) PP_PASS(f(x)) PP_PASS(sep())

#define PP_NIL /* empty */

#define PP_MAP_NEXT_3(...)                                                     \
    PP_MAP_INVOKE(PP_PASS(PP_ARG2(__VA_ARGS__)),                               \
                  PP_PASS(PP_ARG3(__VA_ARGS__)),                               \
                  PP_PASS(PP_ARG4(__VA_ARGS__)))                               \
    PP_DEFER1(PP_PASS(PP_ARG1(__VA_ARGS__)))                                   \
    (PP_PASS(PP_ARG2(__VA_ARGS__)),                                            \
     PP_PASS(PP_ARG3(__VA_ARGS__)),                                            \
     PP_PASS(PP_DROP4(__VA_ARGS__)))

#define PP_MAP_NEXT_2(...) PP_MAP_NEXT_3(__VA_ARGS__)

#define PP_MAP_INVOKE_LAST(f, x) PP_PASS(f(x))

#define PP_MAP_NEXT_1(...)                                                     \
    PP_MAP_INVOKE_LAST(PP_PASS(PP_ARG2(__VA_ARGS__)),                          \
                       PP_PASS(PP_ARG4(__VA_ARGS__)))

#define PP_MAP_NEXT_0(...) /* end mapping */

// run foreach mapping... 1st arg must be function/macro with one
// input argument
#define PP_MAP(...) PP_EVAL(PP_MAP_A(__VA_ARGS__))

#define PP_MAP_WITH_ARG_A(...)                                                 \
    PP_CAT(PP_MAP_WITH_ARG_NEXT_, PP_MAP_SWITCH(0, __VA_ARGS__))               \
    (PP_MAP_WITH_ARG_B, __VA_ARGS__)

#define PP_MAP_WITH_ARG_B(...)                                                 \
    PP_CAT(PP_MAP_WITH_ARG_NEXT_, PP_MAP_SWITCH(0, __VA_ARGS__))               \
    (PP_MAP_WITH_ARG_A, __VA_ARGS__)

#define PP_MAP_WITH_ARG_INVOKE(f, sep, arg, x) PP_PASS(f(arg, x)) PP_PASS(sep())

#define PP_MAP_WITH_ARG_NEXT_3(...)                                            \
    PP_MAP_WITH_ARG_INVOKE(PP_PASS(PP_ARG2(__VA_ARGS__)),                      \
                           PP_PASS(PP_ARG3(__VA_ARGS__)),                      \
                           PP_PASS(PP_ARG4(__VA_ARGS__)),                      \
                           PP_PASS(PP_ARG5(__VA_ARGS__)))                      \
    PP_DEFER1(PP_PASS(PP_ARG1(__VA_ARGS__)))                                   \
    (PP_PASS(PP_ARG2(__VA_ARGS__)),                                            \
     PP_PASS(PP_ARG3(__VA_ARGS__)),                                            \
     PP_PASS(PP_ARG4(__VA_ARGS__)),                                            \
     PP_PASS(PP_DROP5(__VA_ARGS__)))

#define PP_MAP_WITH_ARG_INVOKE_LAST(f, arg, x) PP_PASS(f(arg, x))

#define PP_MAP_WITH_ARG_NEXT_2(...)                                            \
    PP_MAP_WITH_ARG_INVOKE_LAST(PP_PASS(PP_ARG2(__VA_ARGS__)),                 \
                                PP_PASS(PP_ARG4(__VA_ARGS__)),                 \
                                PP_PASS(PP_ARG5(__VA_ARGS__)))

#define PP_MAP_WITH_ARG_NEXT_1(...) /* end mapping */

#define PP_MAP_WITH_ARG(...) PP_EVAL(PP_MAP_WITH_ARG_A(__VA_ARGS__))

#endif
