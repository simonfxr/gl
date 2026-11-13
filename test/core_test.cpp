// Keep core first: its construction helpers must not require <memory>.
#include <bl/core.hpp>

#define CTEST_IMPLEMENTATION
#include <ctest.h>

namespace {

template<typename T, typename... Args>
concept constructible_at = requires(T *p, Args &&...args) {
    bl::construct_at(p, bl_fwd(args)...);
};

struct throwing_constructor
{
    throwing_constructor() noexcept(false);
};

static_assert(constructible_at<int>);
static_assert(constructible_at<int, int>);
static_assert(!constructible_at<int, const char *>);
static_assert(!constructible_at<int[2]>);
static_assert(!constructible_at<const int>);
static_assert(!constructible_at<volatile int>);
static_assert(!constructible_at<void>);
static_assert(noexcept(bl::construct_at(static_cast<int *>(nullptr), 42)));
static_assert(!noexcept(
  bl::construct_at(static_cast<throwing_constructor *>(nullptr))));

struct forwarded
{
    bool rvalue;
    explicit forwarded(int &) noexcept : rvalue(false) {}
    explicit forwarded(int &&) noexcept : rvalue(true) {}
};

struct alignas(64) tracked
{
    int *alive;
    int *moves;
    const int value;

    tracked(int &alive, int &moves, int value) noexcept
      : alive(bl::addressof(alive)), moves(bl::addressof(moves)), value(value)
    {
        ++alive;
    }

    tracked(const tracked &) = delete;
    tracked(tracked &&rhs) noexcept
      : alive(rhs.alive), moves(rhs.moves), value(rhs.value)
    {
        ++*alive;
        ++*moves;
    }

    ~tracked() { --*alive; }

    // Construction and address-taking must bypass these overloads.
    static void *operator new(bl::usize, void *) = delete;
    tracked *operator&() noexcept { return nullptr; }
    const tracked *operator&() const noexcept { return nullptr; }
};

int function() { return 42; }

} // namespace

CTEST_GROUP("core")
{
    CTEST("construct_at value initializes scalar storage")
    {
        alignas(int) bl::byte storage[sizeof(int)];
        auto *value = bl::construct_at(reinterpret_cast<int *>(storage));
        ctest_true(static_cast<void *>(value) == static_cast<void *>(storage));
        ctest_equal(0, *value);
        bl::destroy_at(value);
    }

    CTEST("construct_at forwards lvalues and rvalues")
    {
        alignas(forwarded) bl::byte storage[sizeof(forwarded)];
        int input = 42;
        auto *value =
          bl::construct_at(reinterpret_cast<forwarded *>(storage), input);
        ctest_false(value->rvalue);
        bl::destroy_at(value);
        value = bl::construct_at(reinterpret_cast<forwarded *>(storage), 42);
        ctest_true(value->rvalue);
        bl::destroy_at(value);
    }

    CTEST("relocate_at moves once and destroys the source")
    {
        int alive = 0;
        int moves = 0;
        alignas(tracked) bl::byte source_storage[sizeof(tracked)];
        alignas(tracked) bl::byte dest_storage[sizeof(tracked)];
        auto *source = bl::construct_at(
          reinterpret_cast<tracked *>(source_storage), alive, moves, 42);
        ctest_equal(1, alive);
        ctest_true(bl::addressof(*source) == source);
        ctest_true(bl::addressof(static_cast<const tracked &>(*source)) ==
                   source);

        auto *dest = bl::relocate_at(
          source, reinterpret_cast<tracked *>(dest_storage));
        ctest_true(static_cast<void *>(dest) ==
                   static_cast<void *>(dest_storage));
        ctest_equal(42, dest->value);
        ctest_equal(1, alive);
        ctest_equal(1, moves);
        bl::destroy_at(dest);
        ctest_equal(0, alive);
    }

    CTEST("addressof supports functions and cv-qualified objects")
    {
        ctest_true(bl::addressof(function) == &function);
        const volatile int value = 42;
        ctest_true(bl::addressof(value) == &value);
    }
}

CTEST_MAIN()
