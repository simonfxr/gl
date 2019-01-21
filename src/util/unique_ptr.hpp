#ifndef UTIL_UNIQUE_PTR_HPP
#define UTIL_UNIQUE_PTR_HPP

#ifdef ENABLE_STD_UNIQUE_PTR
#    include <memory>

template<typename... Args>
using unique_ptr = std::unique_ptr<Args...>;

#else
#    include <type_traits>

template<typename T>
struct DefaultDeleter
{
    void operator()(T *p) noexcept { delete p; }
};

template<typename T>
struct DefaultDeleter<T[]>
{
    void operator()(T *p) noexcept { delete[] p; }
};

template<typename T, typename U>
struct CompressedPair : private U
{
    T _fst;

    constexpr CompressedPair() = default;
    constexpr CompressedPair(const T &a, const U &b) : U(b), _fst(a) {}

    constexpr CompressedPair(T &&a, U &&b) : U(std::move(b)), _fst(std::move(a))
    {}

    constexpr CompressedPair(const CompressedPair &) = default;
    constexpr CompressedPair(CompressedPair &&) = default;

    constexpr CompressedPair &operator=(const CompressedPair &) = default;
    constexpr CompressedPair &operator=(CompressedPair &&) = default;

    const T &fst() const { return _fst; }
    T &fst() { return _fst; }

    const U &snd() const { return static_cast<const U &>(*this); }
    U &snd() { return static_cast<U &>(*this); }
};

template<typename T, typename Deleter = DefaultDeleter<T>>
struct unique_ptr
{
    constexpr unique_ptr(T *p = nullptr, Deleter d = Deleter{})
      : _impl(p, std::move(d))
    {}

    unique_ptr(const unique_ptr &) = delete;

    constexpr unique_ptr(unique_ptr &&rhs) : _impl(std::move(rhs._impl))
    {
        rhs._impl.fst() = nullptr;
    }

    ~unique_ptr() { reset(); }

    unique_ptr &operator=(const unique_ptr &) = delete;

    constexpr unique_ptr &operator=(unique_ptr &&rhs)
    {
        reset();
        _impl = std::move(rhs._impl);
        rhs._impl.fst() = nullptr;
        return *this;
    }

    void reset(T *p = nullptr)
    {
        if (get())
            _impl.snd()(get());
        _impl.fst() = p;
    }

    constexpr T *get() { return _impl.fst(); }

    constexpr const T *get() const { return _impl.fst(); }

    constexpr explicit operator bool() const { return get() != nullptr; }

private:
    CompressedPair<T *, Deleter> _impl;
};

#endif
#endif
