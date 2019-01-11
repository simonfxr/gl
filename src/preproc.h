#ifndef PREPROC_H
#define PREPROC_H

// mostly from this execellent resource:
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms

#define PP_CAT(a, ...) PP_CAT0(a, __VA_ARGS__)
#define PP_CAT0(a, ...) a##__VA_ARGS__

#define PP_IIF(c) PP_CAT0(PP_IIF_, c)
#define PP_IIF_0(t, ...) __VA_ARGS__
#define PP_IIF_1(t, ...) t

#define PP_COMPL(b) PP_CAT0(PP_COMPL_, b)
#define PP_COMPL_0 1
#define PP_COMPL_1 0

#define PP_BITAND(x) PP_CAT0(PP_BITAND_, x)
#define PP_BITAND_0(y) 0
#define PP_BITAND_1(y) y

#define PP_INC(x) PP_CAT0(PP_INC_, x)
#define PP_INC_0 1
#define PP_INC_1 2
#define PP_INC_2 3
#define PP_INC_3 4
#define PP_INC_4 5
#define PP_INC_5 6
#define PP_INC_6 7
#define PP_INC_7 8
#define PP_INC_8 9
#define PP_INC_9 9

#define PP_DEC(x) PP_CAT0(PP_DEC_, x)
#define PP_DEC_0 0
#define PP_DEC_1 0
#define PP_DEC_2 1
#define PP_DEC_3 2
#define PP_DEC_4 3
#define PP_DEC_5 4
#define PP_DEC_6 5
#define PP_DEC_7 6
#define PP_DEC_8 7
#define PP_DEC_9 8

#define PP_CHECK_N(x, n, ...) n
#define PP_CHECK(...) PP_CHECK_N(__VA_ARGS__, 0, )
#define PP_PROBE(x) x, 1,

#define PP_IS_PAREN(x) PP_CHECK(PP_IS_PAREN_PROBE x)
#define PP_IS_PAREN_PROBE(...) PP_PROBE(~)

#define PP_NOT(x) PP_CHECK(PP_CAT0(PP_NOT_, x))
#define PP_NOT_0 PP_PROBE(~)

#define PP_BOOL(x) PP_COMPL(PP_NOT(x))
#define PP_IF(c) PP_IIF(PP_BOOL(c))

#define PP_EAT(...)
#define PP_EXPAND(...) __VA_ARGS__
#define PP_WHEN(c) PP_IF(c)(PP_EXPAND, PP_EAT)

#define PP_EMPTY()
#define PP_DEFER(id) id PP_EMPTY()
#define PP_OBSTRUCT(...) __VA_ARGS__ PP_DEFER(PP_EMPTY)()
#define PP_EXPAND(...) __VA_ARGS__

#define PP_EVAL(...) PP_EVAL1(PP_EVAL1(PP_EVAL1(__VA_ARGS__)))
#define PP_EVAL1(...) PP_EVAL2(PP_EVAL2(PP_EVAL2(__VA_ARGS__)))
#define PP_EVAL2(...) PP_EVAL3(PP_EVAL3(PP_EVAL3(__VA_ARGS__)))
#define PP_EVAL3(...) PP_EVAL4(PP_EVAL4(PP_EVAL4(__VA_ARGS__)))
#define PP_EVAL4(...) PP_EVAL5(PP_EVAL5(PP_EVAL5(__VA_ARGS__)))
#define PP_EVAL5(...) __VA_ARGS__

#define PP_REPEAT(count, macro, ...)                                           \
    PP_WHEN(count)                                                             \
    (PP_OBSTRUCT(PP_REPEAT_INDIRECT)()(PP_DEC(count), macro, __VA_ARGS__)      \
       PP_OBSTRUCT(macro)(PP_DEC(count), __VA_ARGS__))
#define PP_REPEAT_INDIRECT() PP_REPEAT

#if 0
// An example of using this macro
#define M(i, _) i
EVAL(REPEAT(8, M, ~)) // 0 1 2 3 4 5 6 7
#endif

#define PP_WHILE(pred, op, ...)                                                \
    PP_IF(pred(__VA_ARGS__))                                                   \
    (PP_OBSTRUCT(PP_WHILE_INDIRECT)()(pred, op, op(__VA_ARGS__)), __VA_ARGS__)
#define PP_WHILE_INDIRECT() PP_WHILE

#define PP_PRIMITIVE_COMPARE(x, y)                                             \
    PP_IS_PAREN(PP_COMPARE_##x(PP_COMPARE_##y)(()))

#define PP_IS_COMPARABLE(x) PP_IS_PAREN(PP_CAT(PP_COMPARE_, x)(()))

#define PP_NOT_EQUAL(x, y)                                                     \
    PP_IIF(PP_BITAND(PP_IS_COMPARABLE(x))(PP_IS_COMPARABLE(y)))                \
    (PP_PRIMITIVE_COMPARE, 1 PP_EAT)(x, y)

#define PP_EQUAL(x, y) PP_COMPL(PP_NOT_EQUAL(x, y))

#endif
