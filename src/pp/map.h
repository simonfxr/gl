#ifndef PP_MAP_H
#define PP_MAP_H

#include "pp/basic.h"

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#    include "pp/map_msvc.h"
#else

#    include "pp/is_empty.h"

/**
 * adopted from: https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h
 */

#    define PP_HAS_ARGS(...) PP_NOT(PP_IS_EMPTY(__VA_ARGS__))

/**
 * Macro map/list comprehension. Usage:
 *   PP_MAP(op, sep, ...)
 * Produces a 'sep()'-separated list of the result of op(arg) for each arg.
 *
 * Example Usage:
 *   #define MAKE_HAPPY(x) happy_##x
 *   PP_MAP(MAKE_HAPPY, PP_COMMA, 1, 2, 3)
 *
 * Which expands to:
 *    happy_1 , happy_2 , happy_3
 */
#    define PP_MAP(...)                                                        \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))(PP_EVAL(PP_MAP_INNER(__VA_ARGS__)))
#    define PP_MAP_INNER(op, sep, cur_val, ...)                                \
        op(cur_val) PP_IF(PP_HAS_ARGS(__VA_ARGS__))(                           \
          sep() PP_DEFER2(PP_IMAP_INNER)()(op, sep, ##__VA_ARGS__))
#    define PP_IMAP_INNER() PP_MAP_INNER

/**
 * Macro map/list comprehension with an additional threaded argument. Usage:
 *   PP_MAP(op, sep, arg1, ...)
 * Produces a 'sep()'-separated list of the result of op(arg1, arg) for each
 * arg.
 *
 * Example Usage:
 *   #define MAKE_HAPPY(pref, x) pref##_happy_##x
 *   MAP(MAKE_HAPPY, COMMA, very, 1, 2, 3)
 *
 * Which expands to:
 *    very_happy_1 , very_happy_2 , very_happy_3
 */
#    define PP_MAP_WITH_ARG(...)                                               \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))                                        \
        (PP_EVAL(PP_MAP_WITH_ARG_INNER(__VA_ARGS__)))
#    define PP_MAP_WITH_ARG_INNER(op, sep, arg1, cur_val, ...)                 \
        op(arg1, cur_val) PP_IF(PP_HAS_ARGS(__VA_ARGS__))(sep() PP_DEFER2(     \
          PP_IMAP_WITH_ARG_INNER)()(op, sep, arg1, ##__VA_ARGS__))
#    define PP_IMAP_WITH_ARG_INNER() PP_MAP_WITH_ARG_INNER

/**
 * This is a variant of the MAP macro which also includes as an argument to the
 * operation a valid C variable name which is different for each iteration.
 * Usage:
 *   PP_MAP_WITH_ID(op, sep, ...)
 *
 * Where op is a macro op(val, id) which takes a list value and an ID. This ID
 * will simply be a unary number using the digit "I", that is, I, II, III, IIII,
 * and so on.
 *
 * Example:
 *   #define MAKE_STATIC_VAR(type, name) static type name;
 *   PP_MAP_WITH_ID(MAKE_STATIC_VAR, PP_EMPTY, int, int, int, bool, char)
 *
 * Which expands to:
 *   static int I; static int II; static int III; static bool IIII; static char
 * IIIII;
 */
#    define PP_MAP_WITH_ID(op, sep, ...)                                       \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))                                        \
        (PP_EVAL(PP_MAP_WITH_ID_INNER(op, sep, I, ##__VA_ARGS__)))
#    define PP_MAP_WITH_ID_INNER(op, sep, id, cur_val, ...)                    \
        op(cur_val, id) PP_IF(PP_HAS_ARGS(__VA_ARGS__))(sep() PP_DEFER2(       \
          PP_IMAP_WITH_ID_INNER)()(op, sep, PP_CAT(id, I), ##__VA_ARGS__))
#    define PP_IMAP_WITH_ID_INNER() PP_MAP_WITH_ID_INNER

/**
 * This is a variant of the MAP macro which iterates over pairs rather than
 * singletons.
 *
 * Usage:
 *   PP_MAP_PAIRS(op, sep, ...)
 *
 * Where op is a macro op(val_1, val_2) which takes two list values.
 *
 * Example:
 *
 *   #define MAKE_STATIC_VAR(type, name) static type name;
 *   PP_MAP_PAIRS(MAKE_STATIC_VAR, PP_EMPTY, char, my_char, int, my_int)
 *
 * Which expands to:
 *   static char my_char; static int my_int;
 */
#    define PP_MAP_PAIRS(op, sep, ...)                                         \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))                                        \
        (PP_EVAL(PP_MAP_PAIRS_INNER(op, sep, __VA_ARGS__)))
#    define PP_MAP_PAIRS_INNER(op, sep, cur_val_1, cur_val_2, ...)             \
        op(cur_val_1, cur_val_2) PP_IF(PP_HAS_ARGS(__VA_ARGS__))(              \
          sep() PP_DEFER2(PP_IMAP_PAIRS_INNER)()(op, sep, __VA_ARGS__))
#    define PP_IMAP_PAIRS_INNER() PP_MAP_PAIRS_INNER

/**
 * This is a variant of the MAP macro which iterates over a two-element sliding
 * window.
 *
 * Usage:
 *   PP_MAP_SLIDE(op, last_op, sep, ...)
 *
 * Where op is a macro op(val_1, val_2) which takes the two list values
 * currently in the window. last_op is a macro taking a single value which is
 * called for the last argument.
 *
 * Example:
 *
 *   #define SIMON_SAYS_OP(simon, next) PP_IF(PP_NOT(simon()))(next)
 *   #define SIMON_SAYS_LAST_OP(val) last_but_not_least_##val
 *   #define SIMON_SAYS() 0
 *
 *   PP_MAP_SLIDE(SIMON_SAYS_OP, SIMON_SAYS_LAST_OP, PP_EMPTY, wiggle,
 * SIMON_SAYS, dance, move, SIMON_SAYS, boogie, stop)
 *
 * Which expands to:
 *   dance boogie last_but_not_least_stop
 */
#    define PP_MAP_SLIDE(op, last_op, sep, ...)                                \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))                                        \
        (PP_EVAL(PP_MAP_SLIDE_INNER(op, last_op, sep, __VA_ARGS__)))
#    define PP_MAP_SLIDE_INNER(op, last_op, sep, cur_val, ...)                 \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))                                        \
        (op(cur_val, PP_FIRST(__VA_ARGS__)))                                   \
          PP_IF(PP_NOT(PP_HAS_ARGS(__VA_ARGS__)))(last_op(cur_val))            \
            PP_IF(PP_HAS_ARGS(__VA_ARGS__))(sep() PP_DEFER2(                   \
              PP_IMAP_SLIDE_INNER)()(op, last_op, sep, __VA_ARGS__))
#    define PP_IMAP_SLIDE_INNER() PP_MAP_SLIDE_INNER

/**
 * Strip any excess commas from a set of arguments.
 */
#    define PP_REMOVE_TRAILING_COMMAS(...)                                     \
        PP_MAP(PP_PASS, PP_COMMA, __VA_ARGS__)

#endif
#endif
