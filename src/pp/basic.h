#ifndef PP_BASIC_H
#define PP_BASIC_H

#include "defs.h"

/**
 * adapted from: https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h
 */

#define PP_EVAL(...) PP_EVAL1024(__VA_ARGS__)
#define PP_EVAL1024(...) PP_EVAL512(PP_EVAL512(__VA_ARGS__))
#define PP_EVAL512(...) PP_EVAL256(PP_EVAL256(__VA_ARGS__))
#define PP_EVAL256(...) PP_EVAL128(PP_EVAL128(__VA_ARGS__))
#define PP_EVAL128(...) PP_EVAL64(PP_EVAL64(__VA_ARGS__))
#define PP_EVAL64(...) PP_EVAL32(PP_EVAL32(__VA_ARGS__))
#define PP_EVAL32(...) PP_EVAL16(PP_EVAL16(__VA_ARGS__))
#define PP_EVAL16(...) PP_EVAL8(PP_EVAL8(__VA_ARGS__))
#define PP_EVAL8(...) PP_EVAL4(PP_EVAL4(__VA_ARGS__))
#define PP_EVAL4(...) PP_EVAL2(PP_EVAL2(__VA_ARGS__))
#define PP_EVAL2(...) PP_EVAL1(PP_EVAL1(__VA_ARGS__))
#define PP_EVAL1(...) __VA_ARGS__

#define PP_PASS(...) __VA_ARGS__
#define PP_EMPTY()
#define PP_COMMA() ,
#define PP_SEMI() ;
#define PP_PLUS() +
#define PP_ZERO() 0
#define PP_ONE() 1

#define PP_NIL /*empty*/

#define PP_IGNORE(...) PP_EMPTY()

#define PP_DEFER1(id) id PP_EMPTY()

#define PP_DEFER2(id) id PP_EMPTY PP_EMPTY()()
#define PP_DEFER3(id) id PP_EMPTY PP_EMPTY PP_EMPTY()()()
#define PP_DEFER4(id) id PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY()()()()
#define PP_DEFER5(id) id PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY()()()()()
#define PP_DEFER6(id)                                                          \
    id PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY()()()()()()
#define PP_DEFER7(id)                                                          \
    id PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY                   \
    PP_EMPTY()()()()()()()
#define PP_DEFER8(id)                                                          \
    id PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY PP_EMPTY          \
    PP_EMPTY()()()()()()()()

#define PP_FIRST(a, ...) a
#define PP_SECOND(a, b, ...) b

#define PP_IS_PROBE(...) PP_SECOND(__VA_ARGS__, 0)
#define PP_PROBE() ~, 1

#define PP_NOT(x) PP_IS_PROBE(PP_CAT(PP_NOT_, x))
#define PP_NOT_0 PP_PROBE()

#define PP_BOOL(x) PP_NOT(PP_NOT(x))

#define PP_OR(a, b) PP_CAT3(PP_OR_, a, b)
#define PP_OR_00 0
#define PP_OR_01 1
#define PP_OR_10 1
#define PP_OR_11 1

#define PP_AND(a, b) PP_CAT3(PP_AND_, a, b)
#define PP_AND_00 0
#define PP_AND_01 0
#define PP_AND_10 0
#define PP_AND_11 1

#define PP_IF(c) PP_IIF(PP_BOOL(c))
#define PP_IIF(c) PP_CAT(PP_IIF_, c)
#define PP_IIF_0(...)
#define PP_IIF_1(...) __VA_ARGS__

#define PP_IF_ELSE(c) PP_IIF_ELSE(PP_BOOL(c))
#define PP_IIF_ELSE(c) PP_CAT(PP_IIF_ELSE_, c)
#define PP_IIF_ELSE_0(t, f) f
#define PP_IIF_ELSE_1(t, f) t

#endif
