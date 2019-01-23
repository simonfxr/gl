#ifndef BL_SHARED_PTR_HPP
#define BL_SHARED_PTR_HPP

#include "bl/core.hpp"
#include "bl/new.hpp"
#include "bl/string_view.hpp"
#include "bl/swap.hpp"
#include "err/err.hpp"

namespace bl {

struct ptr_inplace_init_t
{};

struct shared_from_ptr_t
{};

struct enable_shared_from_this_tag
{};

template<typename T>
using shared_from_this_enabled =
  std::is_assignable<enable_shared_from_this_tag, std::decay_t<T>>;

template<typename T>
struct enable_shared_from_this;

struct shared_ref_count
{
    void retain() noexcept { ++strong_count; }

    bool release() noexcept
    {
        if (--strong_count != 0)
            return false;

        if (deleter && object) {
            deleter(object);
            object = nullptr;
        }

        return true;
    }

    void retain_weak() noexcept { ++weak_count; }
    bool release_weak() noexcept { return --weak_count == 0; }

    uint32_t strong_count = 1;
    uint32_t weak_count = 1;
    const destroy_fun_ptr<void> deleter = nullptr;
    void *object = nullptr;

    explicit shared_ref_count(destroy_fun_ptr<void> del) : deleter(del) {}
};

template<typename T>
struct shared_ref_count_of : shared_ref_count
{
    T value;
    template<typename... Args>
    shared_ref_count_of(ptr_inplace_init_t, Args &&... args)
      : shared_ref_count(get_punned_destroy_fun_ptr<T>())
      , value(std::forward<Args>(args)...)
    {
        this->object = const_cast<void *>(static_cast<const void *>(&value));
    }
};

template<typename T, typename... Args>
inline shared_ref_count *
init_shared_ref_count(T *&ptr_dest, Args &&... args)
{
    ASSERT(!ptr_dest);
    auto *p = new shared_ref_count_of<T>(ptr_inplace_init_t{},
                                         std::forward<Args>(args)...);
    ptr_dest = &p->value;
    return p;
}

template<typename T, typename U>
inline shared_ref_count *
init_shared_ref_count(shared_from_ptr_t, U *ptr)
{
    auto p = new shared_ref_count(get_punned_destroy_fun_ptr<T>());
    p->object = const_cast<void *>(static_cast<const void *>(ptr));
    return p;
}

template<typename T>
struct weak_ptr;

template<typename T>
struct shared_ptr
{
    constexpr shared_ptr() = default;
    constexpr shared_ptr(std::nullptr_t) noexcept {}

    template<typename U = T>
    shared_ptr(shared_from_ptr_t tag, U *p) noexcept
      : _ptr(p), _cnt(p ? init_shared_ref_count<T>(tag, p) : nullptr)
    {
        handle_shared_from_this(p);
    }

    shared_ptr(const shared_ptr &x) noexcept : shared_ptr(x, x.get()) {}

    template<typename U>
    shared_ptr(const shared_ptr<U> &x) noexcept : shared_ptr(x, x.get())
    {}

    template<typename U = T>
    shared_ptr(const shared_ptr<U> &r, T *ptr) noexcept
      : _ptr(ptr), _cnt(ptr ? r._cnt : nullptr)
    {
        if (_cnt) {
            _cnt->retain();
            handle_shared_from_this(ptr);
        }
    }

    template<typename U = T>
    shared_ptr(shared_ptr<U> &&r, T *ptr) noexcept
    {
        if (ptr) {
            *this = std::move(r);
            _ptr = ptr;
            handle_shared_from_this(ptr);
        }
    }

    shared_ptr(shared_ptr &&p) : shared_ptr(std::move(p), p.get()) {}

    template<typename U>
    shared_ptr(shared_ptr<U> &&p) : shared_ptr(std::move(p), p.get())
    {}

    template<typename U>
    inline explicit shared_ptr(const weak_ptr<U> &r)
    {
        if (r._cnt && r._cnt->strong_count > 0) {
            _cnt = r._cnt;
            _ptr = r._ptr;
            _cnt->retain();
        }
    }

    template<typename... Args>
    shared_ptr(ptr_inplace_init_t, Args &&... args)
      : _cnt(init_shared_ref_count<T>(_ptr, std::forward<Args>(args)...))
    {
        handle_shared_from_this(_ptr);
    }

    ~shared_ptr() { reset(); }

    template<typename U>
    shared_ptr &operator=(const shared_ptr<U> &r) noexcept
    {
        return assign(r);
    }

    shared_ptr &operator=(const shared_ptr &r) noexcept { return assign(r); }

    shared_ptr &operator=(shared_ptr &&r) noexcept
    {
        return assign(std::move(r));
    }

    template<typename U>
    shared_ptr &operator=(shared_ptr<U> &&r) noexcept
    {
        return assign(std::move(r));
    }

    void reset() noexcept
    {
        if (_cnt && _cnt->release() && _cnt->release_weak())
            delete _cnt;
        _cnt = nullptr;
        _ptr = nullptr;
    }

    explicit operator bool() const { return _ptr != nullptr; }

    constexpr T *get() const { return _ptr; }

    T *operator->() const { return _ptr; }

    T &operator*() const { return *_ptr; }

private:
    T *_ptr = nullptr;
    shared_ref_count *_cnt = nullptr;

    template<typename U>
    friend struct shared_ptr;

    template<typename U>
    friend struct weak_ptr;

    template<typename U>
    shared_ptr &assign(const shared_ptr<U> &r) noexcept
    {
        if (r._cnt)
            r._cnt->retain();
        reset();
        _ptr = r._ptr;
        _cnt = r._cnt;
        return *this;
    }

    template<typename U>
    shared_ptr &assign(shared_ptr<U> &&r) noexcept
    {
        reset();
        _ptr = ::bl::exchange(r._ptr, nullptr);
        _cnt = ::bl::exchange(r._cnt, nullptr);
        return *this;
    }

    template<typename U>
    void handle_shared_from_this(U *ptr)
    {
        if constexpr (shared_from_this_enabled<std::decay_t<U>>::value) {
            if (ptr)
                handle_shared_from_this(
                  const_cast<std::remove_const_t<U> &>(*ptr));
        }
    }

    template<typename U>
    void handle_shared_from_this(enable_shared_from_this<U> &r)
    {
        if (!r._ref._cnt) {
            r._ref = weak_ptr<U>(*this);
        }
    }

public:
#define DEF_REL_OP(ta, tb, op, px, py)                                         \
    template<typename A, typename B>                                           \
    friend bool operator op(const ta x, const tb y) noexcept                   \
    {                                                                          \
        return px op py;                                                       \
    }

#define DEF_REL_OPS(ta, tb, px, py)                                            \
    DEF_REL_OP(ta, tb, ==, px, py)                                             \
    DEF_REL_OP(ta, tb, !=, px, py)                                             \
    DEF_REL_OP(ta, tb, <, px, py)                                              \
    DEF_REL_OP(ta, tb, <=, px, py)                                             \
    DEF_REL_OP(ta, tb, >, px, py)                                              \
    DEF_REL_OP(ta, tb, >=, px, py)

    DEF_REL_OPS(shared_ptr<A> &, shared_ptr<B>, x.get(), y.get())
    DEF_REL_OPS(shared_ptr<A> &, B *, x.get(), y)
    DEF_REL_OPS(A *, shared_ptr<B> &, x, y.get())

#undef DEF_REL_OPS
#undef DEF_REL_OP
};

template<typename T>
struct weak_ptr
{
    constexpr weak_ptr() noexcept = default;

    weak_ptr(const weak_ptr &r) : _ptr(r._ptr), _cnt(r._cnt)
    {
        if (r._cnt)
            r._cnt->retain_weak();
    }

    template<typename U>
    weak_ptr(const weak_ptr<U> &r) : _ptr(r._ptr), _cnt(r._cnt)
    {
        if (r._cnt)
            r._cnt->retain_weak();
    }

    template<typename U>
    weak_ptr(weak_ptr<U> &&r) noexcept
    {
        *this = std::move(r);
    }

    weak_ptr(weak_ptr &&r) noexcept { *this = std::move(r); }

    template<typename U>
    weak_ptr(const shared_ptr<U> &x) : _ptr(x._ptr), _cnt(x._cnt)
    {
        if (_cnt)
            _cnt->retain_weak();
    }

    ~weak_ptr() { reset(); }

    template<typename U>
    weak_ptr &operator=(const weak_ptr<U> &r) noexcept
    {
        return assign(r);
    }

    template<typename U>
    weak_ptr &operator=(weak_ptr<U> &&r) noexcept
    {
        return assign(std::move(r));
    }

    weak_ptr &operator=(const weak_ptr &r) noexcept { return assign(r); }

    weak_ptr &operator=(weak_ptr &&r) noexcept { return assign(std::move(r)); }

    shared_ptr<T> lock() const noexcept { return shared_ptr<T>(*this); }

    bool expired() const noexcept { return _cnt && _cnt->strong_count > 0; }

    void reset()
    {
        if (_cnt && _cnt->release_weak())
            delete _cnt;
        _cnt = nullptr;
        _ptr = nullptr;
    }

private:
    T *_ptr = nullptr;
    shared_ref_count *_cnt = nullptr;

    template<typename U>
    weak_ptr &assign(const weak_ptr<U> &r) noexcept
    {
        if (r._cnt)
            r._cnt->retain_weak();
        reset();
        _cnt = r._cnt;
        _ptr = r._ptr;
        return *this;
    }

    template<typename U>
    weak_ptr &assign(weak_ptr<U> &&r) noexcept
    {
        reset();
        _ptr = exchange(r._ptr, nullptr);
        _cnt = exchange(r._cnt, nullptr);
        return *this;
    }

    template<typename U>
    friend struct weak_ptr;
    template<typename U>
    friend struct shared_ptr;
}; // namespace bl

template<typename T>
struct enable_shared_from_this : enable_shared_from_this_tag
{
    shared_ptr<T> shared_from_this() { return _ref.lock(); }
    shared_ptr<const T> shared_from_this() const { return _ref.lock(); }

    weak_ptr<T> weak_from_this() { return _ref; }
    weak_ptr<const T> weak_from_this() const { return _ref; }

private:
    weak_ptr<T> _ref;

    template<typename U>
    friend struct shared_ptr;
};

template<typename T, typename... Args>
inline constexpr shared_ptr<T>
make_shared(Args &&... args)
{
    return shared_ptr<T>(ptr_inplace_init_t{}, std::forward<Args>(args)...);
}

} // namespace bl

#endif
