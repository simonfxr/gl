#ifndef CTEST_H_
#define CTEST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "./crand.h"

#ifdef __has_builtin
#    define ctest_has_builtin __has_builtin
#else
#    define ctest_has_builtin(...) 0
#endif

#ifdef _MSC_VER
#    define CTEST_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#    define CTEST_NOINLINE __attribute__((noinline))
#else
#    define CTEST_NOINLINE
#endif

#ifdef __cplusplus
#    define CTEST_ATTR_UNUSED [[maybe_unused]]
#elif defined(__GNUC__)
#    define CTEST_ATTR_UNUSED __attribute__((unused))
#else
#    define CTEST_ATTR_UNUSED
#endif

#ifdef __cplusplus
#    define ctest_cast(T, ...) static_cast<T>(__VA_ARGS__)
#    define ctest_null nullptr
#else
#    define ctest_cast(T, ...) __VA_ARGS__
#    define ctest_null NULL
#endif

#ifdef CTEST_IMPLEMENTATION
#    include <stdio.h>
#    include <stdlib.h>
#    ifdef __unix__
#        include <unistd.h>
#        ifdef _POSIX_VERSION
#            include <signal.h>
#        endif
#    endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CTEST_PP_CAT_I(x, y) x##y
#define CTEST_PP_CAT(x, y) CTEST_PP_CAT_I(x, y)

#define CTEST_STRINGIFY_I(x) #x
#define CTEST_STRINGIFY(x) CTEST_STRINGIFY_I(x)

#ifdef __COUNTER__
#    define CTEST_PP_UNIQUE(x) CTEST_PP_CAT(x, __COUNTER__)
#else
#    define CTEST_PP_UNIQUE(x) CTEST_PP_CAT(x, __LINE__)
#endif

typedef struct ctest_suite ctest_suite;
typedef struct ctest_group ctest_group;
typedef struct ctest_testcase ctest_testcase;

typedef void (*ctest_group_func)(ctest_group *, ctest_suite *);

typedef struct
{
    ctest_group_func f;
    const char *name;
    uint64_t rand_seed;
    const char *func;
    int line;
} ctest_group_registration;

struct ctest_group
{
    const ctest_group_registration *reg;
    ctest_suite *suite;
    int outcome[2];
};

struct ctest_testcase
{
    const char *name;
    ctest_group *group;
    crand_rng rng;
    const char *file;
    int line;
    int events;
    int failed;
    int done;
};

struct ctest_suite
{
    ctest_group_registration *groups;
    size_t groups_size;
    size_t groups_cap;

    char **filters;
    size_t filters_size;
    size_t filters_cap;

    uint64_t rand_seed;
    int debug_break;
    int help_mode;
    int list_mode;
    int initialized;

    int outcome[2];
};

uint64_t
ctest_register_group(ctest_group_func f,
                     const char *name,
                     const char *file,
                     int line);

ctest_testcase
ctest_begin_testcase(ctest_group *group,
                     const char *name,
                     const char *file,
                     int line);

void
ctest_finish_testcase(ctest_testcase *testcase);

int
ctest_abort_testcase(ctest_testcase *testcase,
                     const char *reason,
                     const char *expr,
                     const char *file,
                     int line);

void
ctest_debug_break(void);

int
ctest_main(int argc, char *argv[]);

inline int
ctest_testcase_debug_break(ctest_testcase *testcase)
{
    return testcase->group->suite->debug_break;
}

bool
ctest_glob_match(const char *pat, const char *str);

bool
ctest_glob_prefix_of_match(const char *pat, const char *str);

#define CTEST(name)                                                            \
    for (ctest_testcase CTEST_CURRENT_TESTCASE = ctest_begin_testcase(         \
           CTEST_CURRENT_GROUP, (name), __FILE__, __LINE__);                   \
         !CTEST_CURRENT_TESTCASE.done;                                         \
         ctest_finish_testcase(&CTEST_CURRENT_TESTCASE))

#if defined __cplusplus
#    define CTEST_GROUP_(fname, xname, name)                                   \
        void fname(ctest_group *, ctest_suite *);                              \
        static uint64_t xname =                                                \
          ctest_register_group(fname, (name), __FILE__, __LINE__);             \
        void fname(CTEST_ATTR_UNUSED ctest_group *CTEST_CURRENT_GROUP,         \
                   CTEST_ATTR_UNUSED ctest_suite *CTEST_SUITE)
#elif defined __GNUC__
#    define CTEST_GROUP_(fname, xname, name)                                   \
        void fname(ctest_group *, ctest_suite *);                              \
        __attribute__((constructor)) static void xname(void)                   \
        {                                                                      \
            ctest_register_group(fname, (name), __FILE__, __LINE__);           \
        }                                                                      \
        void fname(CTEST_ATTR_UNUSED ctest_group *CTEST_CURRENT_GROUP,         \
                   CTEST_ATTR_UNUSED ctest_suite *CTEST_SUITE)
#else
#    error                                                                     \
      "UNSUPPORTED PLATFORM: requires either c++ compiler or GNU __attribute__((constructor))"
#endif

#define CTEST_GROUP(name)                                                      \
    CTEST_GROUP_(CTEST_PP_UNIQUE(ctest_group_func),                            \
                 CTEST_PP_UNIQUE(ctest_group_func_init),                       \
                 name)

#define CTEST_MAYBE_DEBUG_BREAK()                                              \
    (ctest_testcase_debug_break(&CTEST_CURRENT_TESTCASE) ? ctest_debug_break() \
                                                         : (void) 0)

#define CTEST_ABORT_(before, reason, expr)                                     \
    if ((before,                                                               \
         (CTEST_MAYBE_DEBUG_BREAK(),                                           \
          !ctest_abort_testcase(                                               \
            &CTEST_CURRENT_TESTCASE, reason, expr, __FILE__, __LINE__)))) {    \
    } else                                                                     \
        continue

#define CTEST_NOP(...) ((void) 0)

#define CTEST_ABORT() CTEST_ABORT_(CTEST_NOP(), "abort", ctest_null)

#define CTEST_EVENT_(...) (CTEST_CURRENT_TESTCASE.events++, (__VA_ARGS__))

#ifndef CTEST_REPORT_NOT_EQUAL
#    define CTEST_REPORT_NOT_EQUAL(...) CTEST_NOP()
#endif

#define CTEST_ABORT_NOT_EQUAL(expected, ...)                                   \
    CTEST_ABORT_(CTEST_REPORT_NOT_EQUAL(expected, __VA_ARGS__),                \
                 "not equal",                                                  \
                 #expected " == " #__VA_ARGS__)

#define ctest_true(...)                                                        \
    if (CTEST_EVENT_(__VA_ARGS__)) {                                           \
    } else                                                                     \
        CTEST_ABORT_(CTEST_NOP(), "should be true, but wasn't", #__VA_ARGS__)

#define ctest_false(...)                                                       \
    if (!CTEST_EVENT_(__VA_ARGS__)) {                                          \
    } else                                                                     \
        CTEST_ABORT_(CTEST_NOP(), "should be false, but wasn't", #__VA_ARGS__)

#define ctest_equal(expected, ...)                                             \
    if (CTEST_EVENT_((expected) == (__VA_ARGS__))) {                           \
    } else                                                                     \
        CTEST_ABORT_NOT_EQUAL(expected, __VA_ARGS__)

#define ctest_rng() (&(CTEST_CURRENT_TESTCASE).rng)

#define ctest_rand(type) crand_int(type, ctest_rng())

#define ctest_rand_bytes(buf, size) crand_bytes(ctest_rng(), (buf), (size))

#define CTEST_MAIN()                                                           \
    int main(int argc, char *argv[])                                           \
    {                                                                          \
        return ctest_main(argc, argv);                                         \
    }

#ifdef __cplusplus
}
#endif

#endif // CTEST_H_

#ifdef CTEST_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static ctest_suite suiteg;

static void
ctest_ensure_suite(ctest_suite *suite)
{
    if (suite->initialized)
        return;
    memset(suite, 0, sizeof(*suite));
    suite->rand_seed = 0x8D6BB9AECEA0A63Full;
    suite->initialized = 1;
}

static int
// NOLINTNEXTLINE(misc-no-recursion)
ctest_glob_match_end1(const char *pat, const char *str, int *r)
{
    for (char p, s; (p = *pat) && (s = *str); str++) {
        if (p == '?' || (p != '*' && p == s))
            pat++;
        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition)
        else if (p == '*' && (*r = ctest_glob_match_end1(pat + 1, str, r)) > 0)
            return 1;
        else if (p != '*')
            return *r;
    }
    if ((!*str && !*pat) || !strcmp(pat, "*"))
        return *r = 1;
    if (!*str)
        return *r = 0;
    return *r;
}

static int
ctest_glob_match_end(const char *pat, const char *str)
{
    int r = -1;
    return ctest_glob_match_end1(pat, str, &r);
}

bool
ctest_glob_match(const char *pat, const char *str)
{
    return ctest_glob_match_end(pat, str) > 0;
}

bool
ctest_glob_prefix_of_match(const char *pat, const char *str)
{
    return ctest_glob_match_end(pat, str) >= 0;
}

uint64_t
ctest_register_group(ctest_group_func f,
                     const char *name,
                     const char *file,
                     int line)
{
    ctest_suite *suite = &suiteg;
    ctest_ensure_suite(suite);
    uint64_t rand_seed = crand_strhash(0x17, name);
    ctest_group_registration r = { f, name, rand_seed, file, line };

    if (suite->groups_cap == suite->groups_size) {
        suite->groups_cap = 2 * suite->groups_cap | 1;
        suite->groups = ctest_cast(
          ctest_group_registration *,
          realloc(suite->groups, suite->groups_cap * sizeof *suite->groups));
    }
    suite->groups[suite->groups_size++] = r;

    return 0;
}

ctest_testcase
ctest_begin_testcase(ctest_group *group,
                     const char *name,
                     const char *file,
                     int line)
{
    int disable = 0;

    const ctest_suite *suite = group->suite;
    if (suite->filters_size) {
        disable = 1;
        char *fqn = ctest_cast(
          char *, malloc(strlen(group->reg->name) + strlen(name) + 2));
        sprintf(fqn, "%s.%s", group->reg->name, name);
        for (size_t i = 0; i < suite->filters_size; ++i) {
            if (ctest_glob_match(suite->filters[i], fqn)) {
                disable = 0;
                break;
            }
        }
        free(fqn);
    }

    crand_rng rng = {};
    if (!disable) {
        uint64_t seed = group->suite->rand_seed ^ group->reg->rand_seed;
        rng = crand_derive(seed, crand_strhash(0x13, name));
        fprintf(stderr, "[%s.%s]\n", group->reg->name, name);
    }
    ctest_testcase testcase = { name, group, rng, file, line, 0, 0, disable };
    return testcase;
}

int
ctest_abort_testcase(ctest_testcase *testcase,
                     const char *reason,
                     const char *expr,
                     const char *file,
                     int line)
{
    if (testcase->failed)
        return 1;
    testcase->failed = 1;

    fprintf(stderr,
            "%s:%d:1   [%s.%s] FAIL   %s%s%s\n",
            file,
            line,
            testcase->group->reg->name,
            testcase->name,
            reason,
            expr ? ": " : "",
            expr ? expr : "");

    return 1;
}

void
ctest_finish_testcase(ctest_testcase *testcase)
{
    if (testcase->done)
        return;
    testcase->done = 1;
    bool pass = !testcase->failed;
    testcase->group->outcome[pass]++;
}

static int
ctest_begin_group(ctest_group *group)
{
    const ctest_suite *suite = group->suite;
    int disable = 0;

    if (suite->filters_size) {
        disable = 1;
        char *group_prefix =
          ctest_cast(char *, malloc(strlen(group->reg->name) + 2));
        sprintf(group_prefix, "%s.", group->reg->name);
        for (size_t i = 0; i < suite->filters_size; ++i) {
            if (ctest_glob_prefix_of_match(suite->filters[i], group_prefix)) {
                disable = 0;
                break;
            }
        }
        free(group_prefix);
    }

    if (disable)
        return 0;

    return 1;
}

static void
ctest_finish_group(ctest_group *group)
{
    int num = group->outcome[false] + group->outcome[true];
    group->suite->outcome[false] += group->outcome[false];
    group->suite->outcome[true] += group->outcome[true];
    if (!num) {
        fprintf(
          stderr,
          "[%s] --- WARNING no testcases were run as part of this group\n",
          group->reg->name);
    }
}

typedef struct
{
    int tag;
    char short_opt;
    const char *long_opt;
    const char *arg_name;
    const char *help;
} ctest_arg_desc;

typedef struct
{
    char **argv;
    char **non_opt_argv;
    const ctest_arg_desc *argd;
    size_t nargd;
} ctest_arg_parser;

void
ctest_arg_parser_init(ctest_arg_parser *p,
                      char **argv,
                      const ctest_arg_desc *argd,
                      size_t nargd)
{
    p->argv = argv + 1;
    p->non_opt_argv = argv + 1;
    p->argd = argd;
    p->nargd = nargd;
}

int
ctest_next_option(ctest_arg_parser *p, const ctest_arg_desc **arg, char **val)
{
    int r = -1;
    int slurp = 0;
    while ((*val = *p->argv) != ctest_null) {
        p->argv++;
        if (**val != '-' || slurp) {
            *p->non_opt_argv++ = *val;
            continue;
        }
        if (!strcmp(*val, "--")) {
            slurp = 1;
            continue;
        }

        char short_opt = 0;
        const char *long_opt = ctest_null;

        if (!strncmp("--", *val, 2))
            long_opt = *val + 2;
        else if (strlen(*val) > 2 || **val != '-')
            goto err;
        else
            short_opt = (*val)[1];

        for (size_t i = 0; i < p->nargd; ++i) {
            *arg = &p->argd[i];
            int match = long_opt && (*arg)->long_opt &&
                        !strcmp(long_opt, (*arg)->long_opt);
            match = match || (short_opt && short_opt == (*arg)->short_opt);
            if (!match)
                continue;
            if (!(*arg)->arg_name)
                return 1;
            if (!*p->argv)
                goto err;
            *val = *p->argv++;
            return 1;
        }
    }
    r = 0;
err:
    *p->non_opt_argv = ctest_null;
    return r;
}

enum cmd_arg
{
    ARG_DEBUG_BREAK,
    ARG_SEED,
    ARG_LIST,
    ARG_HELP,
};

// clang-format off
static const ctest_arg_desc ARG_DESC[] = {
    {ARG_DEBUG_BREAK, 'd', "debug-break", ctest_null,   "debug break on test failures"},
    {ARG_SEED,        's', "seed",        "SEED", "set RNG seed"},
    {ARG_LIST,        'l', "list",        "LIST", "instead of running test, only list defined groups"},
    {ARG_HELP,        'h', "help",        ctest_null,   "print help text"},
};
// clang-format on

static int
ctest_parse_args(ctest_suite *suite, char **argvp)
{

    ctest_arg_parser p;
    ctest_arg_parser_init(
      &p, argvp, ARG_DESC, sizeof(ARG_DESC) / sizeof(ARG_DESC[0]));

    char *val = ctest_null;
    const ctest_arg_desc *arg;
    int ret;

    while ((ret = ctest_next_option(&p, &arg, &val)) > 0) {
        switch ((enum cmd_arg) arg->tag) {
        case ARG_DEBUG_BREAK: {
            suite->debug_break = 1;
            break;
        }
        case ARG_SEED: {
            unsigned long long seed;
            if (sscanf(val, "0x%llx", &seed) == 1)
                suite->rand_seed = (uint64_t) seed;
            else
                suite->rand_seed = crand_strhash(0, val);
            break;
        }
        case ARG_LIST: {
            suite->list_mode = 1;
            break;
        }
        case ARG_HELP: {
            suite->help_mode = 1;
            return 1;
        }
        }
    }
    return ret >= 0;
}

int
ctest_main(int argc, char *argv[])
{
    (void) argc;

    ctest_suite *suite = &suiteg;
    ctest_ensure_suite(suite);

    if (!ctest_parse_args(suite, argv))
        return 1;

    for (++argv; *argv; argv++) {
        if (suite->filters_size == suite->filters_cap) {
            suite->filters_cap = 2 * suite->filters_cap | 1;
            suite->filters =
              ctest_cast(char **,
                         realloc(suite->filters,
                                 suite->filters_cap * sizeof *suite->filters));
        }
        suite->filters[suite->filters_size++] = *argv;
    }

    printf("seed=0x%llX\n", (unsigned long long) suite->rand_seed);

    int ret = 0;
    for (size_t i = 0; i < suite->groups_size; ++i) {
        ctest_group_registration *r = &suite->groups[i];
        ctest_group g = { r, suite, {} };
        if (ctest_begin_group(&g)) {
            r->f(&g, suite);
            ret |= !!g.outcome[false];
            ctest_finish_group(&g);
        }
    }

    fprintf(stderr,
            "[] -- PASS %d FAIL %d\n",
            suite->outcome[true],
            suite->outcome[false]);

    return ret;
}

CTEST_NOINLINE
void
ctest_debug_break(void)
{
#if defined _MSC_VER
    __debugbreak;
#elif ctest_has_builtin(__builtin_debugbreak)
    __builtin_debugbreak();
#elif defined __GNUC__ && (defined(__x86_64__) || defined(__i386__))
    __asm__ __volatile__("int3");
#elif defined _POSIX_VERSION && defined SIGTRAP
    raise(SIGTRAP);
#else
    abort();
#endif
}

#ifdef __cplusplus
}
#endif
#endif // CTEST_IMPLEMENTATION
