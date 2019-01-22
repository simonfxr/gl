#ifndef BL_DYN_ARRAY_HPP
#define BL_DYN_ARRAY_HPP

#include "bl/array_view.hpp"
#include "bl/core.hpp"
#include "bl/vector.hpp"

namespace bl {

template<typename T>
struct dyn_array : private bl::vector<T>
{
    using base = bl::vector<T>;
    using bl::vector<T>::vector;
    using base::begin;
    using base::data;
    using base::empty;
    using base::end;
    using base::size;
    using base::operator[];

    template<typename U = T>
    dyn_array(const U *arr, size_t n) : base(arr, arr + n)
    {}

    template<typename U = T>
    dyn_array(array_view<U> arr) : base(arr.begin(), arr.end())
    {}

    template<typename U>
    static dyn_array<T> transfer_from(const U *data, size_t n)
    {
        return dyn_array<T>(data, n);
    }
};

#if 0
template<typename T>
struct bl::dyn_array
{
    explicit bl::dyn_array(size_t n)
      : _data(n > 0 ? new_array<T>(n) : nullptr), _size(n)
    {}

    constexpr bl::dyn_array() noexcept = default;

    template<typename U>
    bl::dyn_array(bl::array_view<U> view)
    {
        *this = view;
    }

    template<typename U>
    bl::dyn_array(Array<U> &&rhs) noexcept
    {
        *this = bl::move(rhs);
    }

    template<typename U>
    bl::dyn_array(const Array<U> &rhs)
    {
        *this = rhs;
    }

    ~bl::dyn_array() { delete_array(_data); }

    void clear()
    {
        delete_array(_data);
        _data = nullptr;
        _size = 0;
    }

    template<typename U>
    bl::dyn_array &operator=(bl::array_view<U> view)
    {
        if (_size == view.size()) {
            copy_array_data(_data, view.data(), view.size());
        } else {
            clear();
            if (view.size() != 0)
                _data = copy_array<T>(view.data(), view.size());
        }
        return *this;
    }

    template<typename U>
    bl::dyn_array &operator=(const Array<U> &rhs)
    {
        return *this = rhs.view();
    }

    template<typename U>
    bl::dyn_array &operator=(Array<U> &&rhs)
    {
        clear();
        _data = rhs._data;
        _size = rhs._size;
        rhs._data = nullptr;
        rhs._size = 0;
        return *this;
    }

    bl::array_view<const T> view() const { return bl::dyn_arrayView<T>{ _data, _size }; }

    bl::array_view<T> view() { return bl::dyn_arrayView<T>{ _data, _size }; }

    operator bl::array_view<const T>() const { return view(); }

    operator bl::array_view<T>() { return view(); }

    HU_FORCE_INLINE const T &operator[](size_t i) const { return _data[i]; }

    HU_FORCE_INLINE T &operator[](size_t i) { return _data[i]; }

    HU_FORCE_INLINE constexpr bool empty() const { return _size != 0; }

    HU_FORCE_INLINE constexpr size_t size() const { return _size; }

    HU_FORCE_INLINE const T *data() const { return _data; }
    HU_FORCE_INLINE T *data() { return _data; }

    HU_FORCE_INLINE const T *begin() const { return _data; }
    HU_FORCE_INLINE const T *end() const { return _data + _size; }

    HU_FORCE_INLINE T *begin() { return _data; }
    HU_FORCE_INLINE T *end() { return _data + _size; }

    static bl::dyn_array<T> transfer_from(T *data, size_t n)
    {
        return bl::dyn_array<T>(data, n);
    }

private:
    T *_data{};
    size_t _size{};

    bl::dyn_array(T *data, size_t n) : _data(data), _size(n) {}
};
#endif

} // namespace bl
#endif
