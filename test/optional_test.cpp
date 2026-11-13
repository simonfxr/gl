#include <bl/optional.hpp>

#include "./testmain.hpp"

namespace {

struct lifetime_counts
{
    int alive{};
    int constructed{};
    int destroyed{};
    int copied{};
    int moved{};
};

struct tracked
{
    lifetime_counts *counts;
    int value;

    tracked(lifetime_counts &counts, int value) noexcept
      : counts(&counts), value(value)
    {
        ++counts.alive;
        ++counts.constructed;
    }

    tracked(const tracked &rhs) noexcept : tracked(*rhs.counts, rhs.value)
    {
        ++counts->copied;
    }

    tracked(tracked &&rhs) noexcept
      : tracked(*rhs.counts, bl::exchange(rhs.value, -1))
    {
        ++counts->moved;
    }

    tracked &operator=(const tracked &rhs) noexcept
    {
        value = rhs.value;
        return *this;
    }

    tracked &operator=(tracked &&rhs) noexcept
    {
        value = bl::exchange(rhs.value, -1);
        return *this;
    }

    ~tracked()
    {
        --counts->alive;
        ++counts->destroyed;
    }
};

} // namespace

CTEST_GROUP("optional")
{
    CTEST("copy constructs nontrivial and empty values")
    {
        lifetime_counts counts;
        {
            bl::optional<tracked> source;
            source.emplace(counts, 42);
            const auto &original = source;
            bl::optional<tracked> copy(original);

            ctest_true(source);
            ctest_true(copy);
            ctest_equal(42, copy->value);
            ctest_equal(2, counts.alive);
            ctest_equal(1, counts.copied);

            source = bl::nullopt;
            ctest_equal(42, copy->value);
            bl::optional<tracked> empty(source);
            ctest_false(empty);
            ctest_equal(1, counts.alive);
            ctest_equal(1, counts.copied);
        }
        ctest_equal(0, counts.alive);
        ctest_equal(counts.constructed, counts.destroyed);
    }

    CTEST("move constructs and destroys the nontrivial source")
    {
        lifetime_counts counts;
        {
            bl::optional<tracked> source;
            source.emplace(counts, 42);
            bl::optional<tracked> moved(bl::move(source));

            ctest_false(source);
            ctest_true(moved);
            ctest_equal(42, moved->value);
            ctest_equal(1, counts.alive);
            ctest_equal(1, counts.moved);
            ctest_equal(1, counts.destroyed);

            bl::optional<tracked> empty(bl::move(source));
            ctest_false(empty);
            ctest_equal(1, counts.moved);
        }
        ctest_equal(0, counts.alive);
        ctest_equal(counts.constructed, counts.destroyed);
    }

    CTEST("copy assignment handles all engagement states")
    {
        lifetime_counts counts;
        {
            bl::optional<tracked> source;
            bl::optional<tracked> target;
            source.emplace(counts, 42);

            target = source;
            ctest_true(source);
            ctest_true(target);
            ctest_equal(42, target->value);
            ctest_equal(2, counts.alive);

            source->value = 17;
            target = source;
            ctest_equal(17, target->value);
            ctest_equal(2, counts.alive);

            source = bl::nullopt;
            target = source;
            ctest_false(target);
            ctest_equal(0, counts.alive);

            target = source;
            ctest_false(target);
            ctest_equal(0, counts.alive);
        }
        ctest_equal(counts.constructed, counts.destroyed);
    }

    CTEST("move assignment handles all engagement states")
    {
        lifetime_counts counts;
        {
            bl::optional<tracked> source;
            bl::optional<tracked> target;
            source.emplace(counts, 42);

            target = bl::move(source);
            ctest_false(source);
            ctest_true(target);
            ctest_equal(42, target->value);
            ctest_equal(1, counts.alive);
            ctest_equal(1, counts.destroyed);

            source.emplace(counts, 17);
            target = bl::move(source);
            ctest_false(source);
            ctest_equal(17, target->value);
            ctest_equal(1, counts.alive);

            target = bl::move(source);
            ctest_false(target);
            ctest_equal(0, counts.alive);

            target = bl::move(source);
            ctest_false(target);
            ctest_equal(0, counts.alive);
        }
        ctest_equal(counts.constructed, counts.destroyed);
    }
}

CTEST_MAIN()
